#include <StdInc.h>
#include "GenericCallable.h"
#include <Helpers/Convert.h>

#include <Feather/Util/Decl.h>

using namespace SprFrontend;

GenericCallable::GenericCallable(Generic* generic)
    : generic_(generic)
    , inst_(nullptr)
{
}

const Location& GenericCallable::location() const
{
    return generic_->location();
}

string GenericCallable::toString() const
{
    ostringstream oss;
    oss << generic_->toString() << "(";
    for ( size_t i=0; i<generic_->paramsCount(); ++i )
    {
        if ( i> 0)
            oss << ", ";
        oss << generic_->param(i)->type();
    }
    oss << ")";
    return oss.str();
}

size_t GenericCallable::paramsCount() const
{
    return generic_->paramsCount();
}

Node* GenericCallable::param(size_t idx) const
{
    return generic_->param(idx);
}

EvalMode GenericCallable::evalMode() const
{
    return Feather::effectiveEvalMode(generic_);
}
bool GenericCallable::isAutoCt() const
{
    return false;
}

ConversionType GenericCallable::canCall(CompilationContext* context, const Location& loc, const NodeVector& args, EvalMode evalMode, bool noCustomCvt)
{
    // Call the base first
    ConversionType res = Callable::canCall(context, loc, args, evalMode, noCustomCvt);
    if ( !res )
        return convNone;

    // Check if we can instantiate the generic with the given arguments (with conversions applied)
    argsWithCvt_ = argsWithConversion();
    ASSERT(!inst_);
    inst_ = generic_->canInstantiate(argsWithCvt_);
    return inst_ ? res : convNone;
}

Node* GenericCallable::generateCall(const Location& loc)
{
    ASSERT(inst_);
    ASSERT(context_);
    Node* res = generic_->instantiateGeneric(loc, context_, argsWithCvt_, inst_);
    res->setContext(context_);
    return res;
}
