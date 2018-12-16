#include <StdInc.h>
#include "SprTypeTraits.h"
#include "DeclsHelpers.h"
#include "Overload.h"
#include "StdDef.h"
#include <NodeCommonsCpp.h>
#include "Utils/cppif/SparrowFrontendTypes.hpp"
#include "Nodes/SprProperties.h"
#include <Helpers/Generics.h>

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Utils/Tuple.hpp"

#include <utility>

namespace SprFrontend {

using namespace Feather;

unique_ptr<IConvertService> g_ConvertService{};

ConversionType combine(ConversionType lhs, ConversionType rhs) {
    if (lhs == convConcept && rhs == convImplicit)
        return convConceptWithImplicit;
    if (rhs == convConcept && lhs == convImplicit)
        return convConceptWithImplicit;
    return worstConv(lhs, rhs);
}

ConversionType worstConv(ConversionType lhs, ConversionType rhs) {
    return (ConversionType)min(lhs, rhs);
}

ConversionType bestConv(ConversionType lhs, ConversionType rhs) {
    return (ConversionType)max(lhs, rhs);
}

ConversionResult::ConversionResult() {}

ConversionResult::ConversionResult(ConversionType convType)
    : convType_(convType) {}

ConversionResult::ConversionResult(
        ConversionType convType, ConvAction action, const Nest_SourceCode* sourceCode)
    : convType_(convType)
    , sourceCode_(sourceCode) {

    if (convType != convNone && action.first != ActionType::none)
        convertActions_.push_back(action);
}
ConversionResult::ConversionResult(const ConversionResult& nextConv, ConversionType convType,
        ConvAction action, const Nest_SourceCode* sourceCode)
    : convType_(combine(convType, nextConv.conversionType()))
    , sourceCode_(sourceCode) {

    if (convType != convNone) {
        // Check if sourceCode matches
        if (sourceCode) {
            if (nextConv.sourceCode() && nextConv.sourceCode() != sourceCode) {
                convType_ = convNone;
                return;
            }
        } else
            sourceCode_ = nextConv.sourceCode();

        // Combine the actions
        convertActions_.reserve(nextConv.convertActions_.size() + 1);
        if (action.first != ActionType::none)
            convertActions_.emplace_back(action);
        convertActions_.insert(convertActions_.end(), nextConv.convertActions_.begin(),
                nextConv.convertActions_.end());
    }
}

void ConversionResult::addConversion(
        ConversionType convType, ConvAction action, const Nest_SourceCode* sourceCode) {
    // Combine the conversion types
    convType_ = combine(convType_, convType);

    if (convType != convNone) {
        // Check if sourceCode matches
        if (sourceCode) {
            if (sourceCode_ && sourceCode_ != sourceCode) {
                convType_ = convNone;
                return;
            }
        }

        // Add the action to the back
        if (action.first != ActionType::none)
            convertActions_.emplace_back(action);
    }
}

void ConversionResult::addResult(ConversionResult cvt) {
    if (cvt.sourceCode()) {
        if (sourceCode_ && sourceCode_ != cvt.sourceCode()) {
            convType_ = convNone;
            return;
        }
    }
    convType_ = combine(convType_, cvt.conversionType());
    const auto& nextAct = cvt.convertActions();
    convertActions_.insert(convertActions_.end(), nextAct.begin(), nextAct.end());
}

//! Apply one conversion action to the given node
Node* applyOnce(Node* src, ConvAction action) {
    Type destT = action.second;
    switch (action.first) {
    case ActionType::none:
        return src;
    case ActionType::modeCast:
        return src; // Nothing to do when changing mode
    case ActionType::dereference:
        return Feather_mkMemLoad(src->location, src);
    case ActionType::bitcast:
        return Feather_mkBitcast(src->location, Feather_mkTypeNode(src->location, destT), src);
    case ActionType::makeNull:
        return Feather_mkNull(src->location, Feather_mkTypeNode(src->location, destT));
    case ActionType::addRef: {
        Type srcT = removeCatOrRef(TypeWithStorage(destT));
        Node* var = Feather_mkVar(
                src->location, StringRef("$tmpForRef"), Feather_mkTypeNode(src->location, srcT));
        Node* varRef = Feather_mkVarRef(src->location, var);
        Node* store = Feather_mkMemStore(src->location, src, varRef);
        Node* cast =
                Feather_mkBitcast(src->location, Feather_mkTypeNode(src->location, destT), varRef);
        return Feather_mkNodeList(src->location, fromIniList({var, store, cast}));
    }
    case ActionType::customCvt: {
        EvalMode destMode = destT.mode();
        Node* destClass = destT.referredNode();
        Node* refToClass = createTypeNode(
                src->context, src->location, Feather_getDataType(destClass, 0, modeRt));
        return Feather_mkChangeMode(src->location,
                mkFunApplication(src->location, refToClass, fromIniList({src})), destMode);
    }
    }
}

Node* ConversionResult::apply(Node* src) const {
    // If there is no conversion, return the original node
    if (convType_ == convNone)
        return src;

    auto ctx = src->context;

    // If there is a series of conversions, apply them in chain
    for (ConvAction action : convertActions_) {
        ASSERT(src);
        src = applyOnce(src, action);
    }
    // If the original node had a context, set it to the result
    if (ctx)
        Nest_setContext(src, ctx);
    return src;
}

Node* ConversionResult::apply(CompilationContext* context, Node* src) const {
    auto res = apply(src);
    Nest_setContext(res, context);
    return res;
}

namespace {

//! Default implementation of the convert service
struct ConvertService : IConvertService {

    ConversionResult checkConversion(CompilationContext* context, Type srcType, Type destType,
            ConversionFlags flags = flagsDefault) final;
    ConversionResult checkConversion(
            Node* arg, Type destType, ConversionFlags flags = flagsDefault) final;

private:
    using KeyType = std::tuple<Type, Type, int, const Nest_SourceCode*>;

    //! Cache of all the conversions tried so far
    unordered_map<KeyType, ConversionResult> conversionMap_;

    //! Method that checks for available conversions; use the cache for speeding up search.
    //! It always returns an entry in our cache; therefore it's safe to return by const ref.
    const ConversionResult& cachedCheckConversion(
            CompilationContext* context, int flags, Type srcType, Type destType);

    //! Checks all possible conversions (uncached)
    ConversionResult checkConversionImpl(
            CompilationContext* context, int flags, Type srcType, Type destType);

    //! Checks all possible conversions between types of the same mode
    bool checkConversionSameMode(ConversionResult& res, CompilationContext* context, int flags,
            TypeWithStorage srcType, TypeWithStorage destType);

    //! Checks the conversion to a concept (from data-like or other concept)
    bool checkConversionToConcept(ConversionResult& res, CompilationContext* context, int flags,
            TypeWithStorage srcType, TypeWithStorage destType);

    //! Checks the conversion between data-like types
    bool checkDataConversion(ConversionResult& res, CompilationContext* context, int flags,
            TypeWithStorage srcType, TypeWithStorage destType);

    //! Bring the number of references for the source type to the given value.
    //! Add all the conversions to 'res'. Returns false if conversion is impossible.
    //! The source type must be a data-like type.
    bool adjustReferences(ConversionResult& res, TypeWithStorage src, int destKind, int destNumRef,
            const char* destDesc, bool canAddRef);
};

ConversionResult ConvertService::checkConversion(
        CompilationContext* context, Type srcType, Type destType, ConversionFlags flags) {
    return cachedCheckConversion(context, flags, srcType, destType);
}

ConversionResult ConvertService::checkConversion(Node* arg, Type destType, ConversionFlags flags) {
    ASSERT(arg);
    Type srcType = Nest_computeType(arg);
    if (!srcType)
        return {};
    ASSERT(destType);

    return cachedCheckConversion(arg->context, flags, srcType, destType);
}

ConversionResult ConvertService::checkConversionImpl(
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
            src0 = ConceptType::get(ConceptType(srcS).decl(), 0, srcS.mode());
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

bool ConvertService::checkConversionSameMode(ConversionResult& res, CompilationContext* context,
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

bool ConvertService::checkConversionToConcept(ConversionResult& res, CompilationContext* context,
        int flags, TypeWithStorage src, TypeWithStorage dest) {
    ASSERT(src);
    ASSERT(dest);

    // Case 1: data-like -> concept (concept)
    if (Feather::isDataLikeType(src)) {

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
            isOk = g_ConceptsService->conceptIsFulfilled(concept, src);
        }
        // If we have a generic, check if the type is generated from the generic
        else if (concept.kind() == nkSparrowDeclGenericDatatype) {
            isOk = g_ConceptsService->typeGeneratedFromGeneric(concept, src);
        }
        if (!isOk)
            return false;

        // Conversion is possible
        res.addConversion(convConcept);
        return true;
    }

    // Case 2: concept -> concept
    if (src.kind() == typeKindConcept) {
        if (src.numReferences() != dest.numReferences())
            return false;

        // Iteratively search the base concept to find our dest type
        while (src != dest) {
            Nest::NodeHandle conceptNode = ConceptType(src).decl();
            if (!conceptNode)
                return false;
            ConceptType baseType = g_ConceptsService->baseConceptType(conceptNode);
            if (!baseType || baseType == src)
                return false; // Not found; cannot convert
            src = baseType.changeMode(src.mode(), conceptNode.location());
        }

        res.addConversion(convDirect);
        return true;
    }

    return false;
}

bool ConvertService::checkDataConversion(ConversionResult& res, CompilationContext* context,
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

// DataType == 0, ConstType == 1, MutableType == 2, TempType == 3
int typeKindToIndex(int typeKind) { return typeKind - typeKindData; }

bool isCategoryType(int typeKind) {
    return typeKind == typeKindConst || typeKind == typeKindMutable || typeKind == typeKindTemp;
}

TypeWithStorage changeCat(TypeWithStorage src, int typeKind, bool addRef = false) {
    TypeWithStorage base;
    int srcTK = src.kind();
    if (srcTK == typeKindData)
        base = src.numReferences() > 0 && !addRef ? removeRef(src) : src;
    else if (srcTK == typeKindConst)
        base = ConstType(src).base();
    else if (srcTK == typeKindMutable)
        base = MutableType(src).base();
    else if (srcTK == typeKindTemp)
        base = TempType(src).base();

    if (typeKind == typeKindConst)
        return ConstType::get(base);
    else if (typeKind == typeKindMutable)
        return MutableType::get(base);
    else if (typeKind == typeKindTemp)
        return TempType::get(base);
    return src;
}

bool ConvertService::adjustReferences(ConversionResult& res, TypeWithStorage src, int destKind,
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

    constexpr ConvType conversions[4][4] = {
            {direct, addCat, none, none},          // from plain
            {removeCat, direct, none, none},       // from const
            {removeCat, catCast, direct, catCast}, // from mutable
            {removeCat, catCast, catCast, direct}  // from temp
    };
    //  to:  plain,     const,  mutable, temp

    int srcIdx = typeKindToIndex(src.kind());
    int destIdx = typeKindToIndex(destKind);
    ConvType conv = conversions[srcIdx][destIdx];
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

const ConversionResult& ConvertService::cachedCheckConversion(
        CompilationContext* context, int flags, Type srcType, Type destType) {

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

} // namespace

void setDefaultConvertService() {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    g_ConvertService.reset(new ConvertService);
}

ostream& operator<<(ostream& os, ConversionType ct) {
    switch (ct) {
    case convNone:
        os << "None";
        break;
    case convCustom:
        os << "Custom";
        break;
    case convConceptWithImplicit:
        os << "ConceptWithImplicit";
        break;
    case convConcept:
        os << "Concept";
        break;
    case convImplicit:
        os << "Implicit";
        break;
    case convDirect:
        os << "Direct";
        break;
    }
    return os;
}
ostream& operator<<(ostream& os, ActionType act) {
    switch (act) {
    case ActionType::none:
        os << "none";
        break;
    case ActionType::modeCast:
        os << "modeCast";
        break;
    case ActionType::dereference:
        os << "dereference";
        break;
    case ActionType::bitcast:
        os << "bitcast";
        break;
    case ActionType::makeNull:
        os << "makeNull";
        break;
    case ActionType::addRef:
        os << "addRef";
        break;
    case ActionType::customCvt:
        os << "customCvt";
        break;
    default:
        os << "?";
    }
    return os;
}
ostream& operator<<(ostream& os, const ConversionResult& cvt) {
    os << cvt.conversionType() << " - [";
    bool first = true;
    for (auto act : cvt.convertActions()) {
        if (first)
            first = false;
        else
            os << " + ";
        os << act.first << "(" << act.second << ")";
    }
    os << "]";
    return os;
}
} // namespace SprFrontend
