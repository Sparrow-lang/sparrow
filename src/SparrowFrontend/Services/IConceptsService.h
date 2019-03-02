#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Generics.hpp"

namespace SprFrontend {

//! The interface for the service that deals with checking concepts.
//! Used so that we can easily mock and replace this service.
struct IConceptsService {
    virtual ~IConceptsService() {}

    //! Check if the given concept is fulfilled by the given type
    virtual bool conceptIsFulfilled(ConceptDecl concept, Type type) = 0;
    //! Check if the given type was generated from the given generic
    //! This will make generics behave like concepts
    virtual bool typeGeneratedFromGeneric(GenericDatatype genericDatatype, Type type) = 0;

    //! Get the base concept type
    virtual ConceptType baseConceptType(ConceptDecl concept) = 0;
};

//! The convert service instance that we are using across the Sparrow compiler
extern unique_ptr<IConceptsService> g_ConceptsService;

} // namespace SprFrontend