#include <StdInc.h>
#include "SparrowFrontend/Services/Concepts/ConceptsServiceImpl.h"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"

#include "Nest/Utils/Profiling.h"

namespace SprFrontend {

bool ConceptsServiceImpl::conceptIsFulfilled(ConceptDecl concept, Type type) {
    PROFILING_ZONE();

    ASSERT(concept);
    auto instSet = concept.instantiationsSet();

    // We only support data-like types to fulfil concepts
    // TODO (types): Expand this to all datatypes
    if (!Feather::isDataLikeType(type))
        return false;

    if (!concept.isSemanticallyChecked() || !instSet)
        REP_INTERNAL(concept.location(), "Invalid concept");
    ASSERT(instSet);

    NodeHandle typeValue = createTypeNode(concept.context(), concept.location(), type);
    if (!typeValue.semanticCheck())
        return false;

    Instantiation inst =
            canInstantiate(instSet, NodeRange({typeValue}), concept.context()->evalMode);
    return inst;
}

bool ConceptsServiceImpl::typeGeneratedFromGeneric(GenericDatatype genericDatatype, Type type) {
    ASSERT(genericDatatype);
    Feather::StructDecl structDecl = type.referredNode().kindCast<Feather::StructDecl>();
    if (!structDecl)
        return false;

    // Simple check: location & name is the same
    return 0 == Nest_compareLocations(&genericDatatype.location(), &structDecl.location()) &&
           genericDatatype.name() == structDecl.name();
}

ConceptType ConceptsServiceImpl::baseConceptType(ConceptDecl concept) {
    auto baseConceptExp = concept.baseConcept();

    ConceptType res = baseConceptExp ? ConceptType(getType(baseConceptExp)) : ConceptType::get();
    return res;
}

} // namespace SprFrontend
