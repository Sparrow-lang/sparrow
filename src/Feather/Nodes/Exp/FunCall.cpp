#include <StdInc.h>
#include "FunCall.h"
#include <Feather/FeatherNodeCommonsCpp.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>


FunCall::FunCall(const Location& loc, Node* funDecl, DynNodeVector args)
    : DynNode(classNodeKind(), loc, move(args), { (DynNode*) funDecl })
{
}

Node* FunCall::funDecl() const
{
    return data_.referredNodes[0];
}

const DynNodeVector& FunCall::arguments() const
{
    return reinterpret_cast<const DynNodeVector&>(data_.children);
}


void FunCall::dump(ostream& os) const
{
    os << "funCall-" << getName(funDecl()) << "(";
    for ( size_t i=0; i<data_.children.size(); ++i )
    {
        if ( i != 0 )
            os << ", ";
        os << data_.children[i];
    }
    os << ")";
}

void FunCall::doSemanticCheck()
{
    // Function* fun = funDecl();

    // // Make sure the function declaration is has a valid type
    // fun->computeType();

    // // Check argument count
    // if ( data_.children.size() != fun->numParameters() )
    //     REP_ERROR(data_.location, "Invalid function call: expecting %1% parameters, given %2%")
    //         % fun->numParameters() % data_.children.size();

    // // Semantic check the arguments
    // // Also check that their type matches the corresponding type from the function decl
    // bool allParamsAreCtAvailable = true;
    // for ( size_t i=0; i<data_.children.size(); ++i )
    // {
    //     // Semantically check the argument
    //     Nest::semanticCheck(data_.children[i]);
    //     if ( !isCt(data_.children[i]) )
    //         allParamsAreCtAvailable = false;

    //     // Compare types
    //     TypeRef argType = data_.children[i]->type;
    //     TypeRef paramType = fun->getParameter(i)->type();
    //     if ( !isSameTypeIgnoreMode(argType, paramType) )
    //         REP_ERROR(data_.children[i]->location, "Invalid function call: argument %1% is expected to have type %2% (actual type: %3%)")
    //             % (i+1) % paramType % argType;
    // }

    // // CT availability checks
    // EvalMode curMode = data_.context->evalMode();
    // EvalMode calledFunMode = effectiveEvalMode(fun->node());
    // ASSERT(curMode != Nest::modeUnspecified);
    // ASSERT(calledFunMode != Nest::modeUnspecified);
    // if ( calledFunMode == modeCt && curMode != modeCt && !allParamsAreCtAvailable )
    // {
    //     REP_ERROR_NOTHROW(data_.location, "Not all arguments are compile-time, when calling a compile time function");
    //     REP_INFO(fun->location(), "See called function");
    //     REP_ERROR_THROW("Bad mode");
    // }
    // if ( curMode == modeRtCt && calledFunMode == modeRt )
    // {
    //     REP_ERROR_NOTHROW(data_.location, "Cannot call RT functions from RTCT contexts");
    //     REP_INFO(fun->location(), "See called function");
    //     REP_ERROR_THROW("Bad mode");
    // }
    // if ( curMode == modeCt && calledFunMode == modeRt )
    // {
    //     REP_ERROR_NOTHROW(data_.location, "Cannot call a RT function from a CT context");
    //     REP_INFO(fun->location(), "See called function");
    //     REP_ERROR_THROW("Bad mode");
    // }

    // // Get the type from the function decl
    // data_.type = fun->resultType();

    // // Handle autoCt case
    // if ( allParamsAreCtAvailable && data_.type->mode == modeRtCt && fun->hasProperty(propAutoCt) )
    // {
    //     data_.type = changeTypeMode(data_.type, modeCt, data_.location);
    // }

    // // Make sure we yield a type with the right mode
    // data_.type = adjustMode(data_.type, data_.context, data_.location);

    // checkEvalMode(node(), calledFunMode);
}

