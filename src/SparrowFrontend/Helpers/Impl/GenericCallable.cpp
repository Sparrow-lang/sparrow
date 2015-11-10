#include <StdInc.h>
#include "GenericCallable.h"
#include <Helpers/Convert.h>
#include <Helpers/Generics.h>

#include "Feather/Utils/FeatherUtils.hpp"

using namespace SprFrontend;

GenericCallable::GenericCallable(Node* generic)
    : generic_(generic)
    , inst_(nullptr)
{
    ASSERT(isGeneric(generic));
}

const Location& GenericCallable::location() const
{
    return generic_->location;
}

string GenericCallable::toString() const
{
    ostringstream oss;
    oss << Nest_toString(generic_) << "(";
    size_t count = genericParamsCount(generic_);
    for ( size_t i=0; i<count; ++i )
    {
        if ( i> 0)
            oss << ", ";
        oss << genericParam(generic_, i)->type;
    }
    oss << ")";
    return oss.str();
}

size_t GenericCallable::paramsCount() const
{
    return genericParamsCount(generic_);
}

Node* GenericCallable::param(size_t idx) const
{
    return genericParam(generic_, idx);
}

EvalMode GenericCallable::evalMode() const
{
    return Feather::effectiveEvalMode(generic_);
}
bool GenericCallable::isAutoCt() const
{
    return false;
}

ConversionType GenericCallable::canCall(CompilationContext* context, const Location& loc, NodeRange args, EvalMode evalMode, bool noCustomCvt)
{
    // Call the base first
    ConversionType res = Callable::canCall(context, loc, args, evalMode, noCustomCvt);
    if ( !res )
        return convNone;

    // Check if we can instantiate the generic with the given arguments (with conversions applied)
    argsWithCvt_ = argsWithConversion();
    ASSERT(!inst_);
    inst_ = genericCanInstantiate(generic_, all(argsWithCvt_));
    return inst_ ? res : convNone;
}

Node* GenericCallable::generateCall(const Location& loc)
{
    ASSERT(inst_);
    ASSERT(context_);
    Node* res = genericDoInstantiate(generic_, loc, context_, all(argsWithCvt_), inst_);
    Nest_setContext(res, context_);
    return res;
}
