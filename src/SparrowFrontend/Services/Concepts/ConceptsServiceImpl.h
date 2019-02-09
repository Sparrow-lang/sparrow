#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Services/IConceptsService.h"

namespace SprFrontend {

//! The Sparrow implementation of the concepts service
struct ConceptsServiceImpl : IConceptsService {
    bool conceptIsFulfilled(ConceptDecl concept, Type type) final;
    bool typeGeneratedFromGeneric(GenericDatatype genericDatatype, Type type) final;
    ConceptType baseConceptType(ConceptDecl concept) final;
};

} // namespace SprFrontend
