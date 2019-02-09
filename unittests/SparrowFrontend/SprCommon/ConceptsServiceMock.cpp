#include "StdInc.h"
#include "ConceptsServiceMock.hpp"

#include "SparrowFrontend/SparrowFrontendTypes.hpp"

#include "Feather/Utils/cppif/FeatherTypes.hpp"

bool ConceptsServiceMock::conceptIsFulfilled(ConceptDecl concept, Type type) {
    for (auto p : conceptFulfillments_)
        if (p.first == concept && p.second.referredNode() == type.referredNode())
            return true;
    return false;
}
bool ConceptsServiceMock::typeGeneratedFromGeneric(GenericDatatype genericDatatype, Type type) {
    return false;
}
ConceptType ConceptsServiceMock::baseConceptType(ConceptDecl concept) {
    for (auto p : baseConcepts_)
        if (p.first == concept)
            return p.second;
    return nullptr;
}
