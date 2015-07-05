#include <StdInc.h>
#include "FunCall.h"
#include <Feather/FeatherNodeCommonsCpp.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>


FunCall::FunCall(const Location& loc, Function* funDecl, NodeVector args)
    : Node(loc, move(args), { funDecl })
{
}

Function* FunCall::funDecl() const
{
    return (Function*) referredNodes_[0];
}

const NodeVector& FunCall::arguments() const
{
    return children_;
}


void FunCall::dump(ostream& os) const
{
    os << "funCall-" << getName(funDecl()) << "(";
    for ( size_t i=0; i<children_.size(); ++i )
    {
        if ( i != 0 )
            os << ", ";
        os << children_[i];
    }
    os << ")";
}

void FunCall::doSemanticCheck()
{
    Function* fun = funDecl();
    
	// Make sure the function declaration is has a valid type
	fun->computeType();

    // Check argument count
    if ( children_.size() != fun->numParameters() )
        REP_ERROR(location_, "Invalid function call: expecting %1% parameters, given %2%")
            % fun->numParameters() % children_.size();

    // Semantic check the arguments
    // Also check that their type matches the corresponding type from the function decl
    bool allParamsAreCtAvailable = true;
    for ( size_t i=0; i<children_.size(); ++i )
    {
        // Semantically check the argument
        children_[i]->semanticCheck();
        if ( !isCt(children_[i]) )
            allParamsAreCtAvailable = false;

        // Compare types
        TypeRef argType = children_[i]->type();
        TypeRef paramType = fun->getParameter(i)->type();
        if ( !isSameTypeIgnoreMode(argType, paramType) )
            REP_ERROR(children_[i]->location(), "Invalid function call: argument %1% is expected to have type %2% (actual type: %3%)")
                % (i+1) % paramType % argType;
    }

    // CT availability checks
    EvalMode curMode = context_->evalMode();
    EvalMode calledFunMode = effectiveEvalMode(fun);
    ASSERT(curMode != Nest::modeUnspecified);
    ASSERT(calledFunMode != Nest::modeUnspecified);
    if ( calledFunMode == modeCt && curMode != modeCt && !allParamsAreCtAvailable )
    {
        REP_ERROR_NOTHROW(location_, "Not all arguments are compile-time, when calling a compile time function");
        REP_INFO(fun->location(), "See called function");
        REP_ERROR_THROW("Bad mode");
    }
    if ( curMode == modeRtCt && calledFunMode == modeRt )
    {
        REP_ERROR_NOTHROW(location_, "Cannot call RT functions from RTCT contexts");
        REP_INFO(fun->location(), "See called function");
        REP_ERROR_THROW("Bad mode");
    }
    if ( curMode == modeCt && calledFunMode == modeRt )
    {
        REP_ERROR_NOTHROW(location_, "Cannot call a RT function from a CT context");
        REP_INFO(fun->location(), "See called function");
        REP_ERROR_THROW("Bad mode");
    }

    // Get the type from the function decl
    type_ = fun->resultType();

    // Handle autoCt case
    if ( allParamsAreCtAvailable && type_->mode == modeRtCt && fun->hasProperty(propAutoCt) )
    {
        type_ = changeTypeMode(type_, modeCt, location_);
    }

    // Make sure we yield a type with the right mode
    type_ = adjustMode(type_, context_, location_);

    checkEvalMode(this, calledFunMode);
}

