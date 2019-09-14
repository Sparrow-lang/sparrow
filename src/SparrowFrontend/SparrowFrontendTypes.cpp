#include <StdInc.h>

#include "SparrowFrontendTypes.hpp"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Api/TypeKindRegistrar.h"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/Alloc.h"

namespace SprFrontend {

namespace {

using Nest::TypeRef;

const char* getConceptTypeDescription(Node* concept, uint8_t numReferences, EvalMode mode) {
    ostringstream os;
    for (uint8_t i = 0; i < numReferences; ++i)
        os << '@';
    if (concept) {
        os << '#' << Feather_getName(concept);
    } else {
        os << "AnyType";
    }
    if (mode == modeCt)
        os << "/ct";
    return dupString(os.str().c_str());
}

TypeRef getConceptType(Node* conceptOrGeneric, uint8_t numReferences, EvalMode mode) {
    ASSERT(!conceptOrGeneric || conceptOrGeneric->nodeKind == nkSparrowDeclSprConcept ||
            conceptOrGeneric->nodeKind == nkSparrowDeclGenericDatatype);
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindConcept;
    referenceType.mode = mode;
    referenceType.numSubtypes = 0;
    referenceType.numReferences = numReferences;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags = 0;
    referenceType.referredNode = conceptOrGeneric;
    referenceType.description = getConceptTypeDescription(conceptOrGeneric, numReferences, mode);

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t)
        t = Nest_insertStockType(&referenceType);
    return t;
}

TypeRef changeTypeModeConcept(TypeRef type, EvalMode newMode) {
    return getConceptType(type->referredNode, type->numReferences, newMode);
}

} // namespace

int typeKindConcept = -1;

void initSparrowFrontendTypeKinds() {
    typeKindConcept = Nest_registerTypeKind(&changeTypeModeConcept);
}

ConceptType::ConceptType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != typeKindConcept)
        REP_INTERNAL(NOLOC, "ConceptType constructed with other type kind (%1%)") % type;
}

ConceptType ConceptType::get(Nest::NodeHandle decl, int numReferences, Nest::EvalMode mode) {
    return {getConceptType(decl, numReferences, mode)};
}

Nest::NodeHandle ConceptType::decl() const { return referredNode(); }

TypeWithStorage baseType(TypeWithStorage t) {
    while (t && t.numReferences() > 0) {
        int kind = t.kind();
        if (kind == typeKindData)
            t = Feather::DataType::get(t.referredNode(), t.mode());
        else if (kind == typeKindPtr)
            t = Feather::PtrType(t).base();
        else if (kind == typeKindConst)
            t = Feather::ConstType(t).base();
        else if (kind == typeKindMutable)
            t = Feather::MutableType(t).base();
        else if (kind == typeKindTemp)
            t = Feather::TempType(t).base();
        else if (kind == typeKindConcept)
            t = ConceptType::get(t.referredNode(), 0, t.mode());
        else
            REP_INTERNAL(NOLOC, "Cannot get the base type for %1%") % t;
    }
    return t;
}

} // namespace SprFrontend