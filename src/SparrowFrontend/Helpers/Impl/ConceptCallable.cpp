#include <StdInc.h>
#include "ConceptCallable.h"
#include <Helpers/StdDef.h>
#include <Nodes/Decls/SprConcept.h>
#include <SparrowFrontendTypes.h>
#include <Feather/Nodes/FeatherNodes.h>

using namespace SprFrontend;
using namespace Feather;

ConceptCallable::ConceptCallable(SprConcept* concept)
    : concept_(concept)
{
}

const Location& ConceptCallable::location() const
{
    return concept_->location();
}

string ConceptCallable::toString() const
{
    return concept_->toString();
}

size_t ConceptCallable::paramsCount() const
{
    return 1;
}

DynNode* ConceptCallable::param(size_t /*idx*/) const
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


DynNode* ConceptCallable::generateCall(const Location& loc)
{
    ASSERT(concept_);

    // Get the argument, and compile it
    auto argsCvt = argsWithConversion();
    ASSERT(argsCvt.size() == 1);
    DynNode* arg = argsCvt.front();
    ASSERT(arg);
    arg->semanticCheck();

    // Check if the type of the argument fulfills the concept
    bool conceptFulfilled = concept_->isFulfilled(arg->type());
    DynNode* result = mkCtValue(loc, StdDef::typeBool, &conceptFulfilled);
    result->setContext(context_);
    result->semanticCheck();
    return result;
}

