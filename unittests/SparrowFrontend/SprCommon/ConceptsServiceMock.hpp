#pragma once

#include "SparrowFrontend/Helpers/Generics.h"

using Nest::NodeHandle;
using Nest::Type;
using SprFrontend::ConceptDecl;
using SprFrontend::ConceptType;
using SprFrontend::GenericDatatype;

struct ConceptsServiceMock : SprFrontend::IConceptsService {
    bool conceptIsFulfilled(ConceptDecl concept, Type type) final;
    bool typeGeneratedFromGeneric(GenericDatatype genericDatatype, Type type) final;
    ConceptType baseConceptType(ConceptDecl concept) final;

    vector<pair<NodeHandle, Type>> conceptFulfillments_;
    vector<pair<NodeHandle, ConceptType>> baseConcepts_; // concept -> base concept type
};
