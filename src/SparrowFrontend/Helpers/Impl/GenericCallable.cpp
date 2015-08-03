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
    size_t count = genericParamsCount(generic_->node());
    for ( size_t i=0; i<count; ++i )
    {
        if ( i> 0)
            oss << ", ";
        oss << genericParam(generic_->node(), i)->type;
    }
    oss << ")";
    return oss.str();
}

size_t GenericCallable::paramsCount() const
{
    return genericParamsCount(generic_->node());
}

DynNode* GenericCallable::param(size_t idx) const
{
    return (DynNode*) genericParam(generic_->node(), idx);
}

EvalMode GenericCallable::evalMode() const
{
    return Feather::effectiveEvalMode(generic_->node());
}
bool GenericCallable::isAutoCt() const
{
    return false;
}

ConversionType GenericCallable::canCall(CompilationContext* context, const Location& loc, const DynNodeVector& args, EvalMode evalMode, bool noCustomCvt)
{
    // Call the base first
    ConversionType res = Callable::canCall(context, loc, args, evalMode, noCustomCvt);
    if ( !res )
        return convNone;

    // Check if we can instantiate the generic with the given arguments (with conversions applied)
    argsWithCvt_ = argsWithConversion();
    ASSERT(!inst_);
    inst_ = genericCanInstantiate(generic_->node(), fromDyn(argsWithCvt_));
    return inst_ ? res : convNone;
}

DynNode* GenericCallable::generateCall(const Location& loc)
{
    ASSERT(inst_);
    ASSERT(context_);
    Node* res = genericDoInstantiate(generic_->node(), loc, context_, fromDyn(argsWithCvt_), inst_);
    Nest::setContext(res, context_);
    return (DynNode*) res;
}
