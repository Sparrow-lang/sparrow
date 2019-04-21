#include <StdInc.h>
#include "SprTypeTraits.h"
#include "DeclsHelpers.h"
#include "StdDef.h"
#include "SparrowFrontendTypes.hpp"
#include "SparrowFrontend/Services/IConvertService.h"
#include "SparrowFrontend/Services/IOverloadService.h"
#include "NodeCommonsCpp.h"
#include "SprDebug.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"

using namespace SprFrontend;
using namespace Feather;

namespace {
bool getNumericProperties(Type t, int& numBits, bool& isUnsigned, bool& isFloating) {
    if (!t.hasStorage())
        return false;
    Node* cls = t.referredNode();
    ASSERT(cls);

    StringRef nativeName = Nest_getPropertyStringDeref(cls, propNativeName);
    if (nativeName.size() <= 1 || !islower(nativeName.begin[0]))
        return false;

    if (nativeName == "double") {
        numBits = 64;
        isUnsigned = false;
        isFloating = true;
        return true;
    } else if (nativeName == "float") {
        numBits = 32;
        isUnsigned = false;
        isFloating = true;
        return true;
    } else if (nativeName.size() > 1 &&
               (nativeName.begin[0] == 'i' || nativeName.begin[0] == 'u')) {
        try {
            numBits = boost::lexical_cast<int>(nativeName.begin + 1);
            isUnsigned = nativeName.begin[0] == 'u';
            isFloating = false;
            return true;
        } catch (...) {
            return false;
        }
    }
    return false;
}

//! Gets the type value out of the given type node
//! Depending on the second argument may report errors or will be quiet
Type getTypeValueImpl(Node* typeNode, bool reportErrors = false) {
    ASSERT(typeNode);
    if (!Nest_semanticCheck(typeNode)) {
        if (unlikely(reportErrors))
            REP_ERROR(typeNode->location, "Cannot semantically check type node");
        return nullptr;
    }

    // If this is a DeclExp, try to look at the declaration types
    Node* expl = Nest_explanation(typeNode);
    if (expl->nodeKind == nkSparrowExpDeclExp) {
        Type res = nullptr;
        Nest_NodeRange decls = {expl->referredNodes.beginPtr + 1, expl->referredNodes.endPtr};
        for (Node* decl : decls) {
            Type t = nullptr;
            Node* resDecl = resultingDecl(decl);

            // Check if we have a concept or a generic class
            if (resDecl->nodeKind == nkSparrowDeclSprConcept ||
                    resDecl->nodeKind == nkSparrowDeclGenericDatatype)
                t = ConceptType::get(resDecl);
            // Check for a traditional class
            else if (decl->nodeKind == nkSparrowDeclSprDatatype)
                t = decl->type;

            if (t) {
                if (res) {
                    if (unlikely(reportErrors)) {
                        REP_ERROR(typeNode->location, "Multiple declarations found");
                        for (auto d : decls) {
                            if (d)
                                REP_INFO(d->location, "See possible declaration (%1%)") % d;
                        }
                    }
                    return nullptr; // multiple class decls; not a clear type
                }
                res = t;
            }
        }
        if (unlikely(!res && reportErrors)) {
            REP_ERROR(typeNode->location, "No valid type found");
            for (auto d : decls) {
                if (d)
                    REP_INFO(d->location, "See possible declaration (%1%)") % d;
            }
        }
        return res;
    }

    // Is this a Feather type node?
    auto tn = NodeHandle(typeNode).kindCast<Feather::TypeNode>();
    if (tn)
        return tn.givenType();

    Type res = Feather::categoryToRefIfPresent(typeNode->type);

    if (res == StdDef::typeRefType) {
        Node* n = Nest_ctEval(typeNode);
        if (n->nodeKind == nkFeatherExpCtValue) {
            auto** t = Feather_getCtValueData<Type*>(n);
            if (!t || !*t || !**t)
                REP_ERROR_RET(nullptr, typeNode->location, "No type was set for node");
            return **t;
        }
    } else if (res == StdDef::typeType) {
        Node* n = Nest_ctEval(typeNode);
        if (n->nodeKind == nkFeatherExpCtValue) {
            auto* t = Feather_getCtValueData<Type>(n);
            if (!t || !*t)
                REP_ERROR_RET(nullptr, typeNode->location, "No type was set for node");
            return *t;
        }
    }
    if (unlikely(reportErrors))
        REP_ERROR(typeNode->location, "Referenced type is invalid after CT evaluation");
    return nullptr;
}

} // namespace

Type SprFrontend::commonType(CompilationContext* context, Type t1, Type t2) {
    // Check if the types are the same
    if (t1 == t2)
        return t1;

    // Check for numerics
    int numBits1, numBits2;
    bool isUnsigned1, isUnsigned2;
    bool isFloating1, isFloating2;
    if (getNumericProperties(t1, numBits1, isUnsigned1, isFloating1) &&
            getNumericProperties(t2, numBits2, isUnsigned2, isFloating2)) {
        // Floating & non-floating => return the floating point type
        if (isFloating1 && !isFloating2)
            return t1;
        if (!isFloating1 && isFloating2)
            return t2;

        // If the values are too small, return Int
        if (numBits1 < 32 && numBits2 < 32)
            return StdDef::typeInt;

        // Prefer the bigger type to the small type
        if (numBits1 > numBits2)
            return t1;
        if (numBits1 < numBits2)
            return t2;

        // Prefer unsigned, if the types have the same type
        if (numBits1 == numBits2 && isUnsigned1 && !isUnsigned2)
            return t1;
        if (numBits1 == numBits2 && !isUnsigned1 && isUnsigned2)
            return t2;
    }

    // If there is one conversion from one type to another (and not vice-versa) take the less
    // specialized type
    ConversionFlags flags = flagDontAddReference;
    ConversionResult c1 = g_ConvertService->checkConversion(context, t1, t2, flags);
    ConversionResult c2 = g_ConvertService->checkConversion(context, t2, t1, flags);
    if (c1 && !c2) {
        return t2;
    }
    if (!c1 && c2) {
        return t1;
    }

    return StdDef::typeVoid;
}

Type SprFrontend::doDereference1(Nest::NodeHandle arg, Nest::NodeHandle& cvt) {
    cvt = arg;

    // If the base is an expression with a data type, treat this as a data access
    Type t = arg.type();
    if (!t.hasStorage())
        return t;

    // If we have N references apply N-1 dereferencing operations
    for (size_t i = 1; i < t.numReferences(); ++i) {
        cvt = Feather::MemLoadExp::create(arg.location(), cvt);
    }
    return DataType::get(t.referredNode(), 0, t.mode()); // Zero references
}

namespace {
Node* checkDataTypeCtToRtConversion(Node* node) {
    const Location& loc = node->location;
    ASSERT(node->type);

    Type t = node->type;
    Node* cls = t.referredNode();
    if (Feather_effectiveEvalMode(cls) != modeRt)
        REP_INTERNAL(loc, "Cannot convert to RT; datatype %1% doesn't support it") % cls;

    // Check if we have a ct-to-rt ctor
    Node* res = g_OverloadService->selectCtToRtCtor(node);
    if (!res)
        REP_ERROR_RET(nullptr, loc,
                "Cannot convert %1% from CT to RT; make sure 'ctorFromRt' ctor exists") %
                t;

    // Remove the reference from the result
    res = Feather_mkMemLoad(loc, res);

    // Sanity check
    Nest_setContext(res, node->context);
    if (!Nest_computeType(res))
        return nullptr;
    if (res->type->mode != modeRt)
        REP_INTERNAL(loc, "Cannot convert to RT; invalid returned type (%1%)") % t;

    return res;
}
} // namespace

Node* SprFrontend::convertCtToRt(Node* node) {
    const Location& loc = node->location;

    if (!Nest_computeType(node))
        REP_INTERNAL(loc, "Cannot convert null node from CT to RT");

    Type t = node->type;

    if (t.kind() == typeKindVoid) {
        Nest_ctEval(node);
        return Feather_mkNop(loc);
    }

    if (!t.hasStorage())
        REP_ERROR_RET(nullptr, loc, "Cannot convert a non-storage type from CT to RT (%1%)") % t;

    if (t.kind() != typeKindData)
        REP_ERROR_RET(nullptr, loc, "Cannot convert from CT to RT a node of non-data type (%1%)") %
                t;

    if (t.numReferences() > 0)
        REP_ERROR_RET(nullptr, loc, "Cannot convert references from CT to RT (%1%)") % t;

    if (Feather_isBasicNumericType(t) || Type(t).changeMode(modeRt, NOLOC) == StdDef::typeStringRef)
        return Nest_ctEval(node);
    else
        return checkDataTypeCtToRtConversion(node);
}

Type SprFrontend::getType(Node* typeNode) { return getTypeValueImpl(typeNode, true); }

Type SprFrontend::tryGetTypeValue(Node* typeNode) { return getTypeValueImpl(typeNode, false); }

Type SprFrontend::evalTypeIfPossible(Node* typeNode) {
    Type t = tryGetTypeValue(typeNode);
    return t ? t : Type(typeNode->type);
}

Node* SprFrontend::createTypeNode(CompilationContext* context, const Location& loc, Type t) {
    Node* res = Feather_mkCtValueT(loc, StdDef::typeType, &t);
    if (context)
        Nest_setContext(res, context);
    return res;
}

Type SprFrontend::getAutoType(Node* typeNode, int numRefs, EvalMode evalMode) {
    Type t1 = typeNode->type;

    // Nothing to do for non-data-like storage types
    if (!Feather::isDataLikeType(t1))
        return t1;

    Nest::TypeWithStorage t = TypeWithStorage(t1);

    // TODO (types): Rework this; we should receive a concept type and we should apply conversion to
    // it

    // This is a data-like type, so we can directly reduce it to the right datatype
    return DataType::get(t.referredNode(), numRefs, evalMode);
}

bool SprFrontend::isConceptType(Type t) { return t.kind() == typeKindConcept; }

bool SprFrontend::isConceptType(Type t, int& numRefs) {
    if (t.kind() == typeKindConcept) {
        numRefs = int(t.numReferences());
        return true;
    }
    return false;
}

Type SprFrontend::changeRefCount(Type type, int numRef, const Location& loc) {
    ASSERT(type);

    // If we have a category type, get its base
    while (Feather::isCategoryType(type))
        type = Feather_baseType(type);
    // TODO (types): Not sure if this is the right approach

    if (type.kind() == typeKindData)
        type = Feather_getDataType(type.referredNode(), numRef, type.mode());
    else if (type.kind() == typeKindConcept)
        type = ConceptType::get(ConceptType(type).decl(), numRef, type.mode());
    else
        REP_INTERNAL(loc, "Cannot change reference count for type %1%") % type;
    return type;
}

bool SprFrontend::isBitCopiable(Type type) {
    if (!type.hasStorage())
        return false;
    // TODO: Remove this
    if (Feather_isBasicNumericType(type))
        return true;
    return type.referredNode() && type.referredNode().hasProperty(propBitCopiable);
}
