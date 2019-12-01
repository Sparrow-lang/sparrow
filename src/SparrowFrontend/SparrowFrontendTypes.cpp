#include <StdInc.h>

#include "SparrowFrontendTypes.hpp"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

#include "Nest/Api/TypeKindRegistrar.h"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/Alloc.h"

namespace SprFrontend {

namespace {

using Nest::TypeRef;

const char* getConceptTypeDescription(NodeHandle decl, EvalMode mode) {
    ostringstream os;
    if (decl)
        os << '#' << Feather::DeclNode(decl).name();
    else
        os << "AnyType";
    if (mode == modeCt)
        os << "/ct";
    return dupString(os.str().c_str());
}
} // namespace

int typeKindConcept = -1;

void initSparrowFrontendTypeKinds() { typeKindConcept = ConceptType::registerTypeKind(); }

DEFINE_TYPE_COMMON_IMPL(ConceptType, TypeWithStorage)

ConceptType ConceptType::changeTypeModeImpl(ConceptType type, Nest::EvalMode newMode) {
    return ConceptType::get(type.referredNode(), newMode);
}

ConceptType ConceptType::get(Nest::NodeHandle decl, Nest::EvalMode mode) {
    ASSERT(!decl || decl.kind() == nkSparrowDeclSprConcept ||
            decl.kind() == nkSparrowDeclGenericDatatype);
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindConcept;
    referenceType.mode = mode;
    referenceType.numSubtypes = 0;
    referenceType.numReferences = 0;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags = 0;
    referenceType.referredNode = decl;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {

        referenceType.description = getConceptTypeDescription(decl, mode);
        t = Nest_insertStockType(&referenceType);
    }
    return {t};
}

Nest::NodeHandle ConceptType::decl() const { return referredNode(); }

TypeWithStorage baseType(TypeWithStorage t) {
    while (t && t.numReferences() > 0) {
        int kind = t.kind();
        if (kind == typeKindData)
            return t;
        else if (kind == typeKindPtr)
            t = Feather::PtrType(t).base();
        else if (kind == typeKindConst)
            t = Feather::ConstType(t).base();
        else if (kind == typeKindMutable)
            t = Feather::MutableType(t).base();
        else if (kind == typeKindTemp)
            t = Feather::TempType(t).base();
        else if (kind == typeKindConcept)
            return t;
        else
            REP_INTERNAL(NOLOC, "Cannot get the base type for %1%") % t;
    }
    return t;
}

} // namespace SprFrontend