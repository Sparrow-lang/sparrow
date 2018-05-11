#include <StdInc.h>
#include "SprTypeTraits.h"
#include "DeclsHelpers.h"
#include "Overload.h"
#include "StdDef.h"
#include <NodeCommonsCpp.h>
#include <SparrowFrontendTypes.h>
#include <Helpers/Generics.h>

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Utils/Tuple.hpp"

#include <utility>

namespace SprFrontend {

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
        ConversionType convType, ConvAction action, const SourceCode* sourceCode)
    : convType_(convType)
    , sourceCode_(sourceCode) {

    if (convType != convNone && action.first != ActionType::none)
        convertActions_.push_back(action);
}
ConversionResult::ConversionResult(const ConversionResult& nextConv, ConversionType convType,
        ConvAction action, const SourceCode* sourceCode)
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

//! Apply one conversion action to the given node
Node* applyOnce(Node* src, ConvAction action) {
    TypeRef destT = action.second;
    switch (action.first) {
    case ActionType::none:
        return src;
    case ActionType::dereference:
        return Feather_mkMemLoad(src->location, src);
    case ActionType::bitcast:
        return Feather_mkBitcast(src->location, Feather_mkTypeNode(src->location, destT), src);
    case ActionType::makeNull:
        return Feather_mkNull(src->location, Feather_mkTypeNode(src->location, destT));
    case ActionType::addRef: {
        TypeRef srcT = Feather_removeRef(destT);
        Node* var = Feather_mkVar(
                src->location, fromCStr("$tmpForRef"), Feather_mkTypeNode(src->location, srcT));
        Node* varRef = Feather_mkVarRef(src->location, var);
        Node* store = Feather_mkMemStore(src->location, src, varRef);
        Node* cast =
                Feather_mkBitcast(src->location, Feather_mkTypeNode(src->location, destT), varRef);
        return Feather_mkNodeList(src->location, fromIniList({var, store, cast}));
    }
    case ActionType::customCvt: {
        EvalMode destMode = destT->mode;
        Node* destClass = Feather_classForType(destT);
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

    ConversionResult checkConversion(CompilationContext* context, TypeRef srcType, TypeRef destType,
            ConversionFlags flags = flagsDefault) final;
    ConversionResult checkConversion(
            Node* arg, TypeRef destType, ConversionFlags flags = flagsDefault) final;

private:
    using KeyType = std::tuple<TypeRef, TypeRef, int, const SourceCode*>;

    //! Cache of all the conversions tried so far
    unordered_map<KeyType, ConversionResult> conversionMap_;

    //! Method that checks for available conversions; use the cache for speeding up search.
    //! It always returns an entry in our cache; therefore it's safe to return by const ref.
    const ConversionResult& cachedCheckConversion(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    //! Checks all possible conversions (uncached)
    ConversionResult checkConversionImpl(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    //! direct: T -> T
    ConversionResult checkSameType(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    //! direct: T/X -> U/Y, if X!=Y and Y!=ct and (T==ct => noRef(T)==0) and hasMeaning(T/Y)
    //! and T/Y -> U/Y
    ConversionResult checkChangeMode(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    //! concept: #C@N -> Concept@N
    ConversionResult checkConvertToConcept(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    //! concept: Concept1@N -> Concept2@N
    ConversionResult checkConceptToConcept(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    //! direct: lv(T) -> U, if T-> U or Feather_addRef(T) -> U (take best alternative)
    ConversionResult checkLValueToNormal(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    //! implicit: #Null -> U, if isRef(U) and isStorage(U)
    ConversionResult checkNullToReference(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    //! implicit: #C@N -> U, if #C@(N-1) -> U
    ConversionResult checkDereference(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    //! implicit:  #C@0 -> U, if #C@1 -> U
    ConversionResult checkAddReference(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    //! T => U, if U has a conversion ctor for T
    ConversionResult checkConversionCtor(
            CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);
};

ConversionResult ConvertService::checkConversion(
        CompilationContext* context, TypeRef srcType, TypeRef destType, ConversionFlags flags) {
    return cachedCheckConversion(context, flags, srcType, destType);
}

ConversionResult ConvertService::checkConversion(
        Node* arg, TypeRef destType, ConversionFlags flags) {
    ASSERT(arg);
    TypeRef srcType = Nest_computeType(arg);
    if (!srcType)
        return {};
    ASSERT(destType);

    return cachedCheckConversion(arg->context, flags, srcType, destType);
}

ConversionResult ConvertService::checkConversionImpl(
        CompilationContext* context, int flags, TypeRef srcType, TypeRef destType) {
    ASSERT(srcType);
    ASSERT(destType);
    ConversionResult c;

    // cerr << "Checking conversion: " << srcType << " -> " << destType << endl;
    // if (0 == strcmp(srcType->description, "Int/ct") &&
    //         0 == strcmp(destType->description, "Tracer/rtct")) {
    //     const char* s = "put a breakpoint here";
    //     (void)s;
    // }

    // Direct: Types are the same?
    c = checkSameType(context, flags, srcType, destType);
    if (c)
        return c;

    // Direct: Ct to non-ct
    c = checkChangeMode(context, flags, srcType, destType);
    if (c)
        return c;

    bool canCallCvtCtor = 0 == (flags & flagDontCallConversionCtor);
    flags |= flagDontCallConversionCtor; // Don't call conversion ctor in any other conversions

    // Direct: Type with storage to concept
    c = checkConvertToConcept(context, flags, srcType, destType);
    if (c)
        return c;

    // Direct: Concept to another concept
    c = checkConceptToConcept(context, flags, srcType, destType);
    if (c)
        return c;

    // Direct: If we have a l-value, convert into a reference
    c = checkLValueToNormal(context, flags, srcType, destType); // Recursive call
    if (c)
        return c;

    // Implicit: Null to reference
    c = checkNullToReference(context, flags, srcType, destType); // Recursive call
    if (c)
        return c;

    // Implicit: Reference to non reference?
    c = checkDereference(context, flags, srcType, destType); // Recursive call
    if (c)
        return c;

    // Implicit: Non-reference to reference
    if ((flags & flagDontAddReference) == 0) {
        c = checkAddReference(context, flags, srcType, destType); // Recursive call
        if (c)
            return c;
    }

    // Custom: Conversion with conversion constructor
    if (canCallCvtCtor) {
        c = checkConversionCtor(context, flags, srcType, destType); // Recursive call
        if (c)
            return c;
    }

    return {};
}

const ConversionResult& ConvertService::cachedCheckConversion(
        CompilationContext* context, int flags, TypeRef srcType, TypeRef destType) {

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

    // cerr << srcType << " -> " << destType << " : " << res << endl;

    // Put the result in the cache, if not context dependent
    std::get<3>(key) = res.sourceCode();
    auto r = conversionMap_.insert(make_pair(key, res));

    return r.first->second;
}

ConversionResult ConvertService::checkSameType(
        CompilationContext* /*context*/, int /*flags*/, TypeRef srcType, TypeRef destType) {
    return srcType == destType ? convDirect : convNone;
}

ConversionResult ConvertService::checkChangeMode(
        CompilationContext* context, int flags, TypeRef srcType, TypeRef destType) {
    if (!srcType->hasStorage)
        return {};

    EvalMode srcMode = srcType->mode;
    EvalMode destMode = destType->mode;

    // Don't do anything if the modes are the same; can't convert from non-CT to CT
    if (srcMode == destMode || destMode == modeCt)
        return {};

    TypeRef srcTypeNew = Feather_checkChangeTypeMode(srcType, destMode, NOLOC);
    if (srcTypeNew == srcType)
        return {};

    ConvAction action;
    if (srcMode == modeCt) {
        // If we are converting away from CT, make sure we are allowed to do this
        if (!srcType->canBeUsedAtRt)
            return {};

        // Cannot convert references from CT to RT; still we allow l-value conversions
        if (srcTypeNew->typeKind == typeKindLValue) {
            if (srcTypeNew->numReferences > 1)
                return {};
            srcTypeNew = Feather_removeLValueIfPresent(srcTypeNew);
            action = ConvAction(ActionType::dereference, srcTypeNew);
        }
        if (srcTypeNew->numReferences > 0)
            return {};
    }

    const auto& nextConv = cachedCheckConversion(context, flags, srcTypeNew, destType);
    return ConversionResult(nextConv, convDirect, action);
}

ConversionResult ConvertService::checkConvertToConcept(
        CompilationContext* /*context*/, int /*flags*/, TypeRef srcType, TypeRef destType) {
    if (destType->typeKind != typeKindConcept)
        return {};
    if (srcType->typeKind == typeKindConcept || !srcType->hasStorage)
        return {};

    bool isOk = false;

    if (srcType->typeKind != typeKindLValue && srcType->numReferences == destType->numReferences) {
        Node* concept = conceptOfType(destType);
        if (!concept) {
            isOk = true;
        } else {
            // If we have a concept, check if the type fulfills the concept
            if (concept->nodeKind == nkSparrowDeclSprConcept) {
                isOk = g_ConceptsService->conceptIsFulfilled(concept, srcType);
            }

            // If we have a generic, check if the type is generated from the generic
            if (concept->nodeKind == nkSparrowDeclGenericClass) {
                isOk = g_ConceptsService->typeGeneratedFromGeneric(concept, srcType);
            }
        }
    }

    return isOk ? convConcept : convNone;
}

ConversionResult ConvertService::checkConceptToConcept(
        CompilationContext* context, int flags, TypeRef srcType, TypeRef destType) {
    if (srcType->typeKind != typeKindConcept || destType->typeKind != typeKindConcept ||
            srcType->numReferences != destType->numReferences)
        return {};

    Node* srcConcept = conceptOfType(srcType);
    if (!srcConcept)
        return {};

    // If we have a concept, check if the type fulfills the concept
    TypeRef srcBaseConceptType = g_ConceptsService->baseConceptType(srcConcept);
    if (!srcBaseConceptType)
        return {};
    srcBaseConceptType =
            Feather_checkChangeTypeMode(srcBaseConceptType, srcType->mode, srcConcept->location);
    return cachedCheckConversion(context, flags, srcBaseConceptType, destType);
}

ConversionResult ConvertService::checkLValueToNormal(
        CompilationContext* context, int flags, TypeRef srcType, TypeRef destType) {
    if (srcType->typeKind == typeKindLValue && destType->typeKind != typeKindLValue) {
        TypeRef t2 = Feather_lvalueToRef(srcType);
        TypeRef t1 = Feather_removeRef(t2);

        // First check conversion without reference
        const auto& nextConv =
                cachedCheckConversion(context, flags | flagDontAddReference, t1, destType);
        if (nextConv)
            return ConversionResult(nextConv, convDirect, ConvAction(ActionType::dereference, t2));

        // Now try to convert to reference
        const auto& nextConv2 = cachedCheckConversion(context, flags, t2, destType);
        return ConversionResult(nextConv2, convImplicit, ConvAction(ActionType::bitcast, t2));
    }
    return {};
}

ConversionResult ConvertService::checkNullToReference(
        CompilationContext* /*context*/, int /*flags*/, TypeRef srcType, TypeRef destType) {
    if (!StdDef::typeNull || !Feather_isSameTypeIgnoreMode(srcType, StdDef::typeNull) ||
            !destType->hasStorage || destType->numReferences == 0)
        return {};

    return ConversionResult(convImplicit, ConvAction(ActionType::makeNull, destType));
}

ConversionResult ConvertService::checkDereference(
        CompilationContext* context, int flags, TypeRef srcType, TypeRef destType) {
    if (srcType->typeKind != typeKindData || srcType->numReferences == 0)
        return {};

    TypeRef t = Feather_removeRef(srcType);

    const auto& nextConv =
            cachedCheckConversion(context, flags | flagDontAddReference, t, destType);
    return ConversionResult(nextConv, convImplicit, ConvAction(ActionType::dereference, t));
}

ConversionResult ConvertService::checkAddReference(
        CompilationContext* context, int flags, TypeRef srcType, TypeRef destType) {
    if (srcType->typeKind != typeKindData || srcType->numReferences > 0)
        return {};

    TypeRef baseDataType = Feather_addRef(srcType);

    const auto& nextConv =
            cachedCheckConversion(context, flags | flagDontAddReference, baseDataType, destType);
    return ConversionResult(nextConv, convImplicit, ConvAction(ActionType::addRef, baseDataType));
}

ConversionResult ConvertService::checkConversionCtor(
        CompilationContext* context, int flags, TypeRef srcType, TypeRef destType) {
    if (!destType->hasStorage)
        return {};

    Node* destClass = Feather_classForType(destType);
    if (!destClass || !Nest_computeType(destClass))
        return {};

    // Try to convert srcType to lv destClass
    if (!g_OverloadService->selectConversionCtor(context, destClass, destType->mode, srcType))
        return {};

    // Check access
    if (!canAccessNode(destClass, context->sourceCode))
        return {};

    // If the class is not public, store the current source code for this conversion
    // This conversion is not ok in all contexts
    SourceCode* sourceCode = nullptr;
    if (!isPublic(destClass))
        sourceCode = context->sourceCode;

    TypeRef t = destClass->type;
    EvalMode destMode = t->mode;
    if (destMode == modeRt)
        destMode = srcType->mode;
    t = Feather_checkChangeTypeMode(t, destMode, NOLOC);
    TypeRef resType = Feather_getLValueType(t);

    const auto& nextConv =
            cachedCheckConversion(context, flags | flagDontCallConversionCtor, resType, destType);
    return ConversionResult(
            nextConv, convCustom, ConvAction(ActionType::customCvt, resType), sourceCode);
}

} // namespace

void setDefaultConvertService() { g_ConvertService.reset(new ConvertService); }

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
