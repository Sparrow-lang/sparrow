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
#include "Nest/Utils/cppif/SmallVector.hpp"
#include "Nest/Utils/Profiling.h"

#include <utility>

namespace SprFrontend {

using namespace Feather;
using Nest::SmallVector;

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
        if (!isConceptType(destS) && !srcS.canBeUsedAtRt())
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
        else if (isConceptType(srcS)) {
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
    if (isConceptType(destBase)) {
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
    if (isConceptType(srcBase)) {
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

        // Check wrapper types
        bool canAddRef = (flags & flagDontAddReference) == 0;
        if (!checkWrapperTypes(res, src, dest, canAddRef))
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
        // Check wrapper types
        bool canAddRef = (flags & flagDontAddReference) == 0;
        if (!checkWrapperTypes(res, src, dest, canAddRef))
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

        // Finally, check the wrapper types
        bool canAddRef = (flags & flagDontAddReference) == 0;
        if (!checkWrapperTypes(res, resType, dest, canAddRef))
            return false;

        return true;
    }

    return false;
}

namespace {

//! Decompose the type between a base type and a set of wrapper types
void analyzeType(
        TypeWithStorage type, TypeWithStorage& base, SmallVector<int>& wrapperKinds, int& numPtrs) {
    wrapperKinds.clear();
    wrapperKinds.reserve(type.numReferences());
    numPtrs = 0;
    while (true) {
        TypeWithStorage nextType;
        if (type.kind() == typeKindPtr) {
            numPtrs++;
            nextType = PtrType(type).base();
        } else if (type.kind() == typeKindConst)
            nextType = ConstType(type).base();
        else if (type.kind() == typeKindMutable)
            nextType = MutableType(type).base();
        else if (type.kind() == typeKindTemp)
            nextType = TempType(type).base();
        else
            break;

        wrapperKinds.push_back(type.kind());
        ASSERT(nextType);
        type = nextType;
    }
    std::reverse(wrapperKinds.begin(), wrapperKinds.end());
    base = type;
}

//! Apply a wrapper types to the given type.
//! Useful for transforming ConceptTypes to DataTypes of the same shape
TypeWithStorage applyWrapperTypes(TypeWithStorage base, const SmallVector<int>& wrapperKinds) {
    for (auto kind : wrapperKinds) {
        if (kind == typeKindConst)
            base = ConstType::get(base);
        else if (kind == typeKindMutable)
            base = MutableType::get(base);
        else if (kind == typeKindTemp)
            base = TempType::get(base);
        else if (kind == typeKindPtr)
            base = PtrType::get(base);
    }
    return base;
}

enum ElemConvType {
    none = 0,  // no conversion possible
    direct,    // same type
    addPtr,    // add pointer
    removePtr, // remove pointer
    catCast,   // cast between categories (with extra refs)
    addCat,    // plain -> category
    removeCat, // category -> plain
    ptr2Cat,   // ptr -> category
    cat2Ptr,   // category -> ptr
};

// DataType == 0, PtrType == 1, ConstType == 2, MutableType == 3, TempType == 4
int typeKindToIndex(int typeKind) { return typeKind - typeKindData; }

//! Check an elementary casts; looks only at the kinds of the top-most types.
ElemConvType checkElementaryCast(int srcKind, int destKind) {
    int src = typeKindToIndex(srcKind);
    int dest = typeKindToIndex(destKind);
    // clang-format off
    constexpr ElemConvType conversions[5][5] = {
            {direct,    addPtr,    addCat,  none,    addCat},  // from plain
            {removePtr, direct,    ptr2Cat, ptr2Cat, ptr2Cat}, // from ptr
            {removeCat, cat2Ptr,   direct,  none,    none},    // from const
            {removeCat, cat2Ptr,   catCast, direct,  none},    // from mutable
            {removeCat, cat2Ptr,   catCast, none,    direct}   // from temp
    };
    // to:   plain,     ptr,       const,   mutable, temp
    // clang-format on

    return conversions[src][dest];
}

const char* kindToStr(int kind) {
    if (kind == typeKindData)
        return "data";
    if (kind == typeKindPtr)
        return "ptr";
    if (kind == typeKindConst)
        return "const";
    if (kind == typeKindMutable)
        return "mut";
    if (kind == typeKindTemp)
        return "tmp";
    if (kind == 0)
        return "none";
    ASSERT(false);
    return "???";
}

//! A stack of node kinds, from which we can pop the base kinds.
struct KindStack {
    SmallVector<int> kinds;
    int cur{0};

    //! Get the index kind at the given index; returns 0 if going over bounds
    int operator[](int idx) const { return cur + idx < kinds.size() ? kinds[cur + idx] : 0; }

    //! Is the stack empty
    bool empty() const { return cur >= kinds.size(); }

    //! Pop group of kinds (1 is ptr, and we may have another category type)
    //! Any of the two can be 0 -- non existent
    void popGroup(bool& ptr, int& category) {
        int val1 = (*this)[0];
        int val2 = (*this)[1];
        if (val1 == typeKindPtr) {
            ptr = true;
            category = 0;
            cur++;
        } else if (val2 == typeKindPtr) {
            ptr = true;
            category = val1;
            cur += 2;
        } else {
            ASSERT(val2 == 0); // cannot have two consecutive cat types
            ptr = false;
            category = val1;
            cur++;
        }
    }

    //! Consume the first kind from the stack
    void pop() {
        if (!empty())
            cur++;
    }
};

int setPlainIfKindMissing(int kind) { return kind == 0 ? typeKindData : kind; }

} // namespace

bool ConvertServiceImpl::checkWrapperTypes(
        ConversionResult& res, TypeWithStorage src, TypeWithStorage dest, bool canAddRef) {

    bool doDebug = src.description() == StringRef("FooType/ct") &&
                   dest.description() == StringRef("#Concept1/ct");
    doDebug = doDebug || (src.description() == StringRef("Float32") &&
                                 dest.description() == StringRef("Float32 ptr"));
    doDebug = false;
    if (doDebug)
        cerr << "\n" << src.description() << " -> " << dest.description() << "\n";

    // Analyze the two types: figure our their base type and all the wrappers
    static KindStack srcKinds;
    static KindStack destKinds;
    srcKinds.cur = 0;
    destKinds.cur = 0;
    TypeWithStorage srcBase, destBase;
    int srcPtrs = 0;
    int destPtrs = 0;
    analyzeType(src, srcBase, srcKinds.kinds, srcPtrs);
    analyzeType(dest, destBase, destKinds.kinds, destPtrs);

    // Handle the case where the destination is a concept
    // Apply the dest shape on the base source type
    if (destBase.kind() == typeKindConcept && srcBase.kind() != typeKindConcept) {
        dest = applyWrapperTypes(srcBase, destKinds.kinds);
        destBase = srcBase;
    }

    // Check origin
    if (srcBase != destBase)
        return false;

    // First, treat the special case of adding a pointer
    // Allowed conversions:
    //  - T <cat1> -> T <cat0>? ptr <cat2>?, if T <cat1> -> T <cat2>
    // in any other case, adding ptrs is forbidden
    if (srcPtrs == 0 && destPtrs == 1 && canAddRef) {
        auto cat1 = srcKinds[0];
        ASSERT(srcKinds[1] == 0);
        auto k = destKinds[0];
        auto cat2 = k == typeKindPtr ? destKinds[1] : destKinds[2];

        if (doDebug)
            cerr << "  try to add ptr: " << srcBase << " " << kindToStr(cat1) << " -> " << srcBase
                 << " cat? ptr " << kindToStr(cat2) << "\n";

        auto conv = checkElementaryCast(setPlainIfKindMissing(cat1), setPlainIfKindMissing(cat2));
        if (conv == none)
            return false;
        switch (conv) {
        case direct:
            // T <cat> -> T ptr <cat>
            src = addRef(src);
            res.addConversion(convImplicit, ConvAction(ActionType::addRef, src));
            return true;
        case catCast:
            // T <cat1> -> T ptr <cat2>
            src = addRef(src); // => T ptr <cat1>
            res.addConversion(convImplicit, ConvAction(ActionType::addRef, src));
            // now, T ptr <cat1> -> T ptr <cat2>
            res.addConversion(convImplicit, ConvAction(ActionType::bitcast, dest));
            return true;
        case addCat:
        case ptr2Cat:
            // T -> T ptr <cat>
            // Disallow adding two pointers
            return false;
        case removeCat:
        case cat2Ptr:
            // T <cat> -> T ptr
            // bitcast category into pointer
            res.addConversion(convImplicit, ConvAction(ActionType::bitcast, dest));
            return true;
        case addPtr:
        case removePtr:
        default:
            ASSERT(false);
            return false;
        }
    }
    // Cannot add ptrs in any other ptr cardinality
    // TODO: what about T ptr mut -> T ptr ptr ?
    if (srcPtrs < destPtrs)
        return false;

    // First clear up the pointers from both sides -- advance in tandem
    // We match pointer to pointer
    // We check categories at every iteration
    // Note: between pointers we may have at most one cat type, but nothing else.
    int numIterations = std::min(srcPtrs, destPtrs);
    if (doDebug)
        cerr << "  numIterations: " << numIterations << "\n";
    bool needsCast = false;
    for (int i = 0; i < numIterations; i++) {
        // Get the two groups of kinds that we need to compare
        bool srcPtr = false;
        int srcCat = 0;
        srcKinds.popGroup(srcPtr, srcCat);
        bool destPtr = false;
        int destCat = 0;
        destKinds.popGroup(destPtr, destCat);
        if (srcCat == 0)
            srcCat = typeKindData;
        if (destCat == 0)
            destCat = typeKindData;

        if (doDebug) {
            cerr << "    src ptr:" << srcPtr << " cat:" << srcCat << "; dest ptr:" << destPtr
                 << " cat:" << destCat << "\n";
        }

        // Check elementary casts between possible category types
        auto conv = checkElementaryCast(srcCat, destCat);
        if (conv == none)
            return false;

        if (conv != direct)
            needsCast = true;
    }

    // Now the middle part, after the common pointers
    // dest may or may not have a cat left
    // Try to consume everything from dest
    bool shouldAddCat = false;
    bool needsDeref = false;
    bool needsImplicit = false;
    int destKind = setPlainIfKindMissing(destKinds[0]);
    int srcKind = setPlainIfKindMissing(srcKinds[0]);
    auto conv = checkElementaryCast(srcKind, destKind);
    if (doDebug)
        cerr << "  " << kindToStr(srcKind) << " -> " << kindToStr(destKind) << " => " << conv
             << "\n";
    if (conv == none)
        return false;

    switch (conv) {
    case direct:
        // T <cat> <others>* -> U <cat> (where T and U are ref-equivalent)
        srcKinds.pop();
        break;
    case catCast:
        // T <cat1> <others>* -> U <cat2> (where T and U are ref-equivalent)
        needsCast = true;
        needsImplicit = true;
        srcKinds.pop();
        break;
    case addCat:
        // T -> U <cat> (where T and U are ref-equivalent)
        // the source doesn't have anymore ptrs
        if (srcKinds.cur > 0 && destKind == typeKindTemp)
            return false; // Forbid adding 'temp' on ptr types
        ASSERT(srcKinds.empty());
        needsImplicit = true;
        shouldAddCat = true;
        break;
    case ptr2Cat:
        // T ptr <others>* -> U <cat>
        needsImplicit = true;
        needsCast = true;
        srcKinds.pop();
        break;
    case removeCat:
        // T <cat> <others>* -> U
        // needsImplicit = true;
        needsDeref = true;
        srcKinds.pop();
        break;
    case removePtr:
        // T ptr <others>* -> U
        // Don't do anything. Treat this withing general deref
        break;
    case cat2Ptr:
    case addPtr:
    default:
        ASSERT(false);
        return false;
    }

    // Dest should be empty now
    destKinds.pop();
    ASSERT(destKinds.empty());

    // Do we just need to add a category?
    if (shouldAddCat) {
        if (doDebug)
            cerr << "  adding cat => addRef (direct)\n";
        ASSERT(srcKinds.empty());
        res.addConversion(convDirect, ConvAction(ActionType::addRef, dest));
        return true;
    }

    // We may still have some wrapper types in src.
    // Process them in reverse order now
    int remainingPtrs = 0;
    for (int i = 0; /*nothing*/; i++) {
        int kind = srcKinds[i];
        if (kind == 0)
            break;
        if (kind == typeKindPtr)
            remainingPtrs++;
    }
    if (doDebug) {
        cerr << "  num remaining ptrs: " << remainingPtrs << "\n";
        cerr << "  needsDeref: " << needsDeref << "\n";
        cerr << "  needsCast: " << needsCast << "\n";
    }
    for (int i = 0; i < remainingPtrs; i++) {
        src = removeRef(src);
        res.addConversion(convImplicit, ConvAction(ActionType::dereference, src));
    }
    // TODO: remove this
    if (Feather::isCategoryType(src) && src.numReferences() > dest.numReferences()) {
        if (doDebug)
            cerr << "  try extra deref by removing cat (" << src << " -> " << dest << ")\n";
        src = removeCategoryIfPresent(src);
        res.addConversion(convDirect, ConvAction(ActionType::dereference, src));
        // TODO (now): Should we make this convImplicit?
    }
    if (src != dest) {
        if (doDebug)
            cerr << "  bitcast: " << src << " -> " << dest << "\n";
        if (needsCast)
            res.addConversion(needsImplicit ? convImplicit : convDirect,
                    ConvAction(ActionType::bitcast, dest));
        else if (src.numReferences() > dest.numReferences())
            res.addConversion(convImplicit, ConvAction(ActionType::dereference, dest));
        else
            REP_INTERNAL(NOLOC, "Invalid conversion between %1% and %2%") % src % dest;
    }

    if (doDebug)
        cerr << "  res: " << res << "\n";
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
