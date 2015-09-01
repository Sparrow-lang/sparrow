#include <StdInc.h>
#include "ConceptCallable.h"
#include <Helpers/StdDef.h>
#include <SparrowFrontendTypes.h>
#include <Helpers/Generics.h>
#include <Feather/Nodes/FeatherNodes.h>

using namespace SprFrontend;
using namespace Feather;

ConceptCallable::ConceptCallable(Node* concept)
    : concept_(concept)
{
}

const Location& ConceptCallable::location() const
{
    return concept_->location;
}

string ConceptCallable::toString() const
{
    return Nest_toString(concept_);
}

size_t ConceptCallable::paramsCount() const
{
    return 1;
}

Node* ConceptCallable::param(size_t /*idx*/) const
{
    // For a callable concept, we don't have a parameter
    return nullptr;
}

TypeRef ConceptCallable::paramType(size_t /*idx*/) const
{
    return getConceptType();
}

EvalMode ConceptCallable::evalMode() const
{
    return modeCt;
}
bool ConceptCallable::isAutoCt() const
{
    return false;
}


Node* ConceptCallable::generateCall(const Location& loc)
{
    ASSERT(concept_);

    // Get the argument, and compile it
    auto argsCvt = argsWithConversion();
    ASSERT(argsCvt.size() == 1);
    Node* arg = argsCvt.front();
    ASSERT(arg);
    if ( !Nest_semanticCheck(arg) )
        return nullptr;

    // Check if the type of the argument fulfills the concept
    bool conceptFulfilled = conceptIsFulfilled(concept_, arg->type);
    Node* result = mkCtValue(loc, StdDef::typeBool, &conceptFulfilled);
    Nest_setContext(result, context_);
    return Nest_semanticCheck(result);
}

