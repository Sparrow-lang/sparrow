#include <StdInc.h>
#include "Callable.h"
#include <Feather/Util/TypeTraits.h>

using namespace SprFrontend;
using namespace Nest;

Callable::Callable()
    : context_(nullptr)
{
}

TypeRef Callable::paramType(size_t idx) const
{
    Node* p = param(idx);
    ASSERT(p);
    return p->type;
}

ConversionType Callable::canCall(CompilationContext* context, const Location& loc, const NodeVector& args, EvalMode evalMode, bool noCustomCvt)
{
    // Copy the list of arguments; add default values if arguments are missing
    size_t paramsCount = this->paramsCount();
    args_ = args;
    args_.reserve(paramsCount);
    for ( size_t i=args.size(); i<paramsCount; ++i )
    {
        Node* defaultArg = this->paramDefaultVal(i);
        if ( !defaultArg )
            return convNone;        // We have a non-default parameter but we don't have an argument for that
        if ( !Nest_semanticCheck(defaultArg) )  // Make sure this is semantically checked
            return convNone;

        args_.push_back(defaultArg);
    }

    // Do the checks on types
    vector<TypeRef> argTypes(args_.size(), nullptr);
    for ( size_t i=0; i<args_.size(); ++i)
        argTypes[i] = args_[i]->type;
    ConversionType res = canCall(context, loc, argTypes, evalMode, noCustomCvt);
    if ( !res )
        return convNone;

    return res;
}

ConversionType Callable::canCall(CompilationContext* context, const Location& /*loc*/, const vector<TypeRef>& argTypes, EvalMode evalMode, bool noCustomCvt)
{
    // Check argument count
    size_t paramsCount = this->paramsCount();
    if ( paramsCount != argTypes.size() )
        return convNone;

    // Check evaluation mode
    EvalMode thisEvalMode = this->evalMode();
    if ( thisEvalMode == modeRt && (evalMode == modeCt || evalMode == modeRtCt) )
        return convNone;    // Don't call RT-only functions in CT and RTCT contexts
    bool useCt = thisEvalMode == modeCt || evalMode == modeCt;
    if ( thisEvalMode == modeRtCt && isAutoCt() )
    {
        // In autoCt mode, if all the arguments are CT, make a CT call
        useCt = true;
        for ( TypeRef t: argTypes )
        {
            if ( t->mode != modeCt )
            {
                useCt = false;
                break;
            }
        }
    }

    conversions_.resize(paramsCount, convNone);

    ConversionType res = convDirect;
    for ( size_t i=0; i<paramsCount; ++i )
    {
        TypeRef argType = argTypes[i];
        ASSERT(argType);
        TypeRef paramType = this->paramType(i);
        ASSERT(paramType);

        // If we are looking at a CT callable, make sure the parameters are in CT
        if ( paramType->hasStorage && useCt )
            paramType = Feather::changeTypeMode(paramType, modeCt);

        ConversionFlags flags = noCustomCvt ? flagDontCallConversionCtor : flagsDefault;
        conversions_[i] = canConvertType(context, argType, paramType, flags);
        if ( !conversions_[i] )
            return convNone;
        else if ( conversions_[i].conversionType() < res )
            res = conversions_[i].conversionType();
    }

    context_ = context;
    return res;
}

Node* Callable::paramDefaultVal(size_t idx) const
{
    Node* p = param(idx);
    ASSERT(p);
    Node* sprParam = Nest_ofKind(p, nkSparrowDeclSprParameter);
    return sprParam ? sprParam->children[1] : nullptr;
}

NodeVector Callable::argsWithConversion()
{
    NodeVector res(args_.size(), nullptr);
    for ( size_t i=0; i<args_.size(); ++i )
        res[i] = conversions_[i].apply(args_[i]->context, args_[i]);
    return res;
}

void SprFrontend::destroyCallables(Callables& callables)
{
    for ( Callable* c: callables )
    {
        delete c;
    }
    callables.clear();
}
