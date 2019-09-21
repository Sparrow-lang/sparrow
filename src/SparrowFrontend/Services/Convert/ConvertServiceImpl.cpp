#include <StdInc.h>
#include "ConvertServiceImpl.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "SparrowFrontend/NodeCommonsCpp.h"
#include "SparrowFrontend/SparrowFrontendTypes.hpp"
#include "SparrowFrontend/Nodes/SprProperties.h"
#include "SparrowFrontend/Services/IOverloadService.h"
#include "SparrowFrontend/Services/IConceptsService.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Utils/Tuple.hpp"
#include "Nest/Utils/Profiling.h"

#include <utility>

namespace SprFrontend {

using namespace Feather;

ConversionResult ConvertServiceImpl::checkConversion(
        CompilationContext* context, Type srcType, Type destType, ConversionFlags flags) {
    return cachedCheckConversion(context, flags, srcType, destType);
}

ConversionResult ConvertServiceImpl::checkConversion(
        Node* arg, Type destType, ConversionFlags flags) {
    ASSERT(arg);
    Type srcType = Nest_computeType(arg);
    if (!srcType)
        return {};
    ASSERT(destType);

    return cachedCheckConversion(arg->context, flags, srcType, destType);
}

ConversionResult ConvertServiceImpl::checkConversionImpl(
        CompilationContext* context, int flags, Type src, Type dest) {
    ASSERT(src);
    ASSERT(dest);

    // If the types are the same, then we are happy
    if (src == dest)
        return convDirect;

    // If any of the types doesn't have storage, conversion is invalid
    if (!src.hasStorage() || !dest.hasStorage())
        return convNone;

    TypeWithStorage srcS = TypeWithStorage(src);
    TypeWithStorage destS = TypeWithStorage(dest);

    ConversionResult res{convDirect};

    // Check for null conversions
    if (StdDef::clsNull && srcS.referredNode() == StdDef::clsNull) {
        if (isDataLikeType(destS) && destS.numReferences() > 0 &&
                destS.referredNode() != StdDef::clsNull) {
            res.addConversion(convImplicit, ConvAction(ActionType::makeNull, destS));
            return res;
        }
    }

    // Do the types have the same mode?
    if (srcS.mode() == destS.mode()) {
        if (!checkConversionSameMode(res, context, flags, srcS, destS))
            return {};
    } else {
        // The only supported mode conversion is CT->RT
        if (srcS.mode() != modeCt || destS.mode() != modeRt)
            return {};
        // For datatypes conversion, the source type must be usable at RT
        // TODO (types): check MyRange/ct -> #Range, where MyRange is ct-only
        if (destS.kind() != typeKindConcept && !srcS.canBeUsedAtRt())
            return {};

        // Disallow conversion of references
        // @T/ct -> @T is disallowed
        // Allowed: T/ct -> @T, @T/ct -> T, T/ct -> T mut, T/ct mut -> T
        int srcRefsBase = srcS.numReferences();
        int destRefsBase = destS.numReferences();
        if (isCategoryType(srcS))
            --srcRefsBase;
        if (isCategoryType(destS))
            --destRefsBase;
        if (srcRefsBase != 0 && destRefsBase != 0)
            return {};

        // If the types have different modes, then we split our conversion in the following:
        //  a) convert 'src' to reach zero-references
        //  b) apply CT-to-RT conversion
        //  c) covert result to 'dest'

        TypeWithStorage src0; // same as 'src', with with zero references
        if (isDataLikeType(srcS))
            src0 = removeAllRefs(srcS);
        else if (srcS.kind() == typeKindConcept) {
            src0 = ConceptType::get(ConceptType(srcS).decl(), srcS.mode());
        } else
            return {};

        // a) Remove all references from source
        if (srcS != src0)
            if (!checkConversionSameMode(res, context, flags, srcS, src0))
                return {};

        // b) remove CT
        src0 = src0.changeMode(modeRt, NOLOC);
        res.addConversion(convDirect, ConvAction(ActionType::modeCast, src0));

        // c) convert src0 to dest
        if (src0 != destS)
            if (!checkConversionSameMode(res, context, flags, src0, destS))
                return {};
    }

    return res;
}

bool ConvertServiceImpl::checkConversionSameMode(ConversionResult& res, CompilationContext* context,
        int flags, TypeWithStorage src, TypeWithStorage dest) {
    ASSERT(src);
    ASSERT(dest);

    TypeWithStorage destBase = baseType(dest);

    // Is the destination is a concept?
    if (destBase.kind() == typeKindConcept) {
        return checkConversionToConcept(res, context, flags, src, dest);
    }

    // Treat data-like to data-like conversions
    if (isDataLikeType(dest) && isDataLikeType(src)) {
        return checkDataConversion(
                res, context, flags, TypeWithStorage(src), TypeWithStorage(dest));
    }

    return false;
}

bool ConvertServiceImpl::checkConversionToConcept(ConversionResult& res,
        CompilationContext* context, int flags, TypeWithStorage src, TypeWithStorage dest) {
    ASSERT(src);
    ASSERT(dest);

    TypeWithStorage srcBase = baseType(src);
    TypeWithStorage destBase = baseType(dest);

    // Case 1: concept -> concept
    if (srcBase.kind() == typeKindConcept) {
        if (src.numReferences() != dest.numReferences())
            return false;

        // Iteratively search the base concept to find our dest type
        src = srcBase;
        while (src != destBase) {
            ConceptDecl conceptNode = ConceptDecl(ConceptType(src).decl());
            if (!conceptNode)
                return false;
            ConceptType baseType = g_ConceptsService->baseConceptType(conceptNode);
            if (!baseType || baseType == src)
                return false; // Not found; cannot convert
            src = baseType.changeMode(src.mode(), conceptNode.location());
        }

        // TODO (types): Fix this after fixing references

        res.addConversion(convDirect);
        return true;
    }

    // Case 2: data-like -> concept (concept)
    else if (Feather::isDataLikeType(src)) {

        // Treat the destination type kind as data-like
        int destTypeKind = dest.kind();
        if (destTypeKind == typeKindConcept)
            destTypeKind = typeKindData;

        // Adjust references
        bool canAddRef = (flags & flagDontAddReference) == 0;
        if (!adjustReferences(
                    res, src, destTypeKind, dest.numReferences(), dest.description(), canAddRef))
            return false;

        bool isOk = false;
        Nest::NodeHandle concept = dest.referredNode();
        if (!concept)
            isOk = true;
        // If we have a concept, check if the type fulfills the concept
        else if (concept.kind() == nkSparrowDeclSprConcept) {
            isOk = g_ConceptsService->conceptIsFulfilled(ConceptDecl(concept), src);
        }
        // If we have a generic, check if the type is generated from the generic
        GenericDatatype genericDatatype = concept.kindCast<GenericDatatype>();
        if (genericDatatype) {
            isOk = g_ConceptsService->typeGeneratedFromGeneric(genericDatatype, src);
        }
        if (!isOk)
            return false;

        // Conversion is possible
        res.addConversion(convConcept);
        return true;
    }

    return false;
}

bool ConvertServiceImpl::checkDataConversion(ConversionResult& res, CompilationContext* context,
        int flags, TypeWithStorage src, TypeWithStorage dest) {

    // Precondition: we only support datatype-like conversions
    if (!isDataLikeType(removeCategoryIfPresent(src)))
        return false;

    // Case 1: The datatypes have the same decl
    if (dest.referredNode() == src.referredNode()) {
        // Adjust references
        bool canAddRef = (flags & flagDontAddReference) == 0;
        if (!adjustReferences(
                    res, src, dest.kind(), dest.numReferences(), dest.description(), canAddRef))
            return false;

        res.addConversion(convDirect);
        return true;
    }

    // Case 2: Custom conversions
    else if (0 == (flags & flagDontCallConversionCtor)) {

        Node* destClass = dest.referredNode();
        if (!destClass)
            return false;

        // If the destination class is not marked as convert, bail out
        // This saves us some processing time
        if (!Nest_hasProperty(destClass, propConvert))
            return false;

        if (!Nest_computeType(destClass))
            return false;

        // Try to convert srcType to mut destClass
        if (!g_OverloadService->selectConversionCtor(context, destClass, dest.mode(), src))
            return false;

        // Check access
        if (!canAccessNode(destClass, context->sourceCode))
            return false;

        // If the class is not public, store the current source code for this conversion
        // This conversion is not ok in all contexts
        Nest_SourceCode* sourceCode = nullptr;
        if (!isPublic(destClass))
            sourceCode = context->sourceCode;

        TypeWithStorage t = destClass->type;
        EvalMode destMode = t.mode();
        if (destMode == modeRt)
            destMode = src.mode();
        t = t.changeMode(destMode, NOLOC);
        TypeWithStorage resType = Feather::isCategoryType(t) ? t : MutableType::get(t);

        res.addConversion(convCustom, ConvAction(ActionType::customCvt, resType), sourceCode);

        // Ensure we have the right number of references & category
        bool canAddRef = (flags & flagDontAddReference) == 0;
        if (!adjustReferences(
                    res, resType, dest.kind(), dest.numReferences(), dest.description(), canAddRef))
            return false;

        return true;
    }

    return false;
}

namespace {

// DataType == 0, PtrType == 1, ConstType == 2, MutableType == 3, TempType == 4
int typeKindToIndex(int typeKind) { return typeKind - typeKindData; }

bool isCategoryType(int typeKind) {
    return typeKind == typeKindConst || typeKind == typeKindMutable || typeKind == typeKindTemp;
}

TypeWithStorage changeCat(TypeWithStorage src, int typeKind, bool addRef = false) {
    TypeWithStorage base;
    int srcTK = src.kind();
    if (srcTK == typeKindData)
        base = src;
    else if (srcTK == typeKindPtr)
        base = !addRef ? PtrType(src).base() : src;
    else if (srcTK == typeKindConst)
        base = ConstType(src).base();
    else if (srcTK == typeKindMutable)
        base = MutableType(src).base();
    else if (srcTK == typeKindTemp)
        base = TempType(src).base();
    ASSERT(base);

    if (typeKind == typeKindConst)
        return ConstType::get(base);
    else if (typeKind == typeKindMutable)
        return MutableType::get(base);
    else if (typeKind == typeKindTemp)
        return TempType::get(base);
    else if (typeKind == typeKindPtr)
        return PtrType::get(base);
    return src;
}

} // namespace

bool ConvertServiceImpl::adjustReferences(ConversionResult& res, TypeWithStorage src, int destKind,
        int destNumRef, const char* destDesc, bool canAddRef) {
    int srcRefsBase = src.numReferences();
    int destRefsBase = destNumRef;
    if (isCategoryType(src.kind()))
        --srcRefsBase;
    if (isCategoryType(destKind))
        --destRefsBase;

    int srcNumRef = src.numReferences();

    enum ConvType {
        none = 0,  // no conversion possible
        direct,    // same type
        catCast,   // cast between categories (with extra refs)
        addCat,    // plain -> category
        removeCat, // category -> plain
    };

    // clang-format off
    constexpr ConvType conversions[5][5] = {
            {direct,    direct,    addCat,  none,    addCat},  // from plain
            {direct,    direct,    addCat,  addCat,  addCat},  // from ptr
            {removeCat, removeCat, direct,  none,    none},    // from const
            {removeCat, removeCat, catCast, direct,  none},    // from mutable
            {removeCat, removeCat, catCast, none,    direct}   // from temp
    };
    // to:   plain,     ptr,       const,   mutable, temp
    // clang-format on

    int srcIdx = typeKindToIndex(src.kind());
    int destIdx = typeKindToIndex(destKind);
    ConvType conv = conversions[srcIdx][destIdx];
    // TODO (types): Remove this after finalizing mutables
    // A reference can be converted to mutable
    if (srcIdx == 0 && destIdx == 3 && srcRefsBase > destRefsBase) {
        for (int i = destRefsBase; i < srcRefsBase - 1; i++) {
            src = removeCatOrRef(src);
            res.addConversion(convImplicit, ConvAction(ActionType::dereference, src));
        }
        ASSERT(src.numReferences() == destRefsBase + 1);
        src = MutableType::get(removeRef(src));
        res.addConversion(convImplicit, ConvAction(ActionType::bitcast, src));
        return true;
    }
    if (conv == none)
        return false;

    int numDerefs = srcRefsBase - destRefsBase;

    // If we are removing category, check the best way to get rid of the category
    if (conv == removeCat) {
        if (srcNumRef < destNumRef)
            // If we don't have enough references at source, conversion failed
            return false;
        else if (srcNumRef == destNumRef) {
            // Convert the category type to reference
            // With this, we have the exact number of references needed
            src = TypeWithStorage(categoryToRefIfPresent(src));
            ++srcRefsBase;
            ++numDerefs;
            res.addConversion(convImplicit, ConvAction(ActionType::bitcast, src));
        } else /*srcNumRef > destNumRef*/ {
            // We have more references on the source
            // Just remove the category
            src = removeCategoryIfPresent(src);
            res.addConversion(convDirect, ConvAction(ActionType::dereference, src));
        }
    }

    // Need to add reference? (exactly one reference)
    // Allowed: T -> @T, T const -> @T const, T mut -> @T const, etc.
    // Disallowed: @T -> @@T
    if (canAddRef && srcRefsBase == 0 && destRefsBase == 1) {
        src = addRef(src);
        res.addConversion(convImplicit, ConvAction(ActionType::addRef, src));
        numDerefs = 0;
    }

    // If we are here, we can't add anymore references
    if (numDerefs < 0)
        return false;

    // If we are adding category, try to avoid deref + addRef
    if (conv == addCat && numDerefs > 0) {
        --numDerefs;
        res.addConversion(convImplicit);
    }

    // Need to remove some references?
    for (int i = 0; i < numDerefs; i++) {
        src = removeCatOrRef(src);
        res.addConversion(convImplicit, ConvAction(ActionType::dereference, src));
    }

    // Ensure that we cast to the right category at the end
    // This happens for catCast, addCat, but also when we change references
    if (src.kind() != destKind) {
        if (src.numReferences() == 0)
            res.addConversion(
                    convDirect, ConvAction(ActionType::addRef, changeCat(src, destKind, true)));
        else {
            bool addRef = src.numReferences() < destNumRef;
            if (addRef)
                res.addConversion(
                        convDirect, ConvAction(ActionType::addRef, changeCat(src, destKind, true)));
            else
                res.addConversion(convDirect,
                        ConvAction(ActionType::bitcast, changeCat(src, destKind, false)));
        }
    }

    return true;
}

const ConversionResult& ConvertServiceImpl::cachedCheckConversion(
        CompilationContext* context, int flags, Type srcType, Type destType) {
    PROFILING_ZONE();

    // Try to find the conversion in the map -- first, try without a source code
    KeyType key(srcType, destType, flags, nullptr);
    auto it = conversionMap_.find(key);
    if (it != conversionMap_.end())
        return it->second;
    // Now try with a source code
    key = KeyType(srcType, destType, flags, context->sourceCode);
    it = conversionMap_.find(key);
    if (it != conversionMap_.end())
        return it->second;

    // Compute the value normally
    ConversionResult res = checkConversionImpl(context, flags, srcType, destType);

    // cout << srcType << " -> " << destType << " : " << res << endl;
    // cout << srcType << " -> " << destType << " (" << flags << ") :" << res << endl;

    // Put the result in the cache, if not context dependent
    std::get<3>(key) = res.sourceCode();
    auto r = conversionMap_.insert(make_pair(key, res));

    return r.first->second;
}

} // namespace SprFrontend
