#include <StdInc.h>
#include "SprReturn.h"
#include <NodeCommonsCpp.h>
#include <Helpers/Convert.h>
#include <Helpers/CommonCode.h>
#include <Helpers/DeclsHelpers.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/TypeTraits.h>

using namespace SprFrontend;
using namespace Nest;
using namespace Feather;


SprReturn::SprReturn(const Location& loc, DynNode* exp)
    : DynNode(classNodeKind(), loc, {exp})
{
}

void SprReturn::doSemanticCheck()
{
    ASSERT(children_.size() == 1);
    DynNode* exp = children_[0];

    // Get the parent function of this return
    Function* parentFun = getParentFun(context_);
    if ( !parentFun )
        REP_ERROR(location_, "Return found outside any function");

    // Compute the result type of the function
    TypeRef resType = nullptr;
    DynNode* resultParam = getResultParam(parentFun);
    if ( resultParam ) // Does this function have a result param?
    {
        resType = removeRef(resultParam->type());
        ASSERT(!parentFun->resultType()->hasStorage); // The function should have void result
    }
    else
    {
        resType = parentFun->resultType();
    }
    ASSERT(resType);

    // Check match between return expression and function result type
    ConversionResult cvt = convNone;
    if ( exp )
    {
        exp->semanticCheck();
        if ( !resType->hasStorage && exp->type() == resType )
        {
            setExplanation(mkNodeList(location_, { exp, mkReturn(location_) }));
            return;
        }
        else
        {
            cvt = canConvert(exp, resType);
        }
        if ( !cvt )
            REP_ERROR(exp->location(), "Cannot convert return expression (%1%) to %2%") % exp->type() % resType;
    }
    else
    {
        if ( parentFun->resultType()->typeKind != typeKindVoid )
            REP_ERROR(location_, "You must return something in a function that has non-Void result type");
    }

    // Build the explanation of this node
    if ( resultParam )
    {
        // Create a ctor to construct the result parameter with the expression received
        const Location& l = resultParam->location();
        DynNode* thisArg = mkMemLoad(l, mkVarRef(l, resultParam));
        thisArg->setContext(context_);
        DynNode* action = createCtorCall(l, context_, thisArg, exp);
        if ( !action )
            REP_ERROR(exp->location(), "Cannot construct return type object %1% from %2%") % exp->type() % resType;

        if ( action )
        {
            setExplanation(mkNodeList(location_, { action, mkReturn(location_, nullptr)}));
        }
    }
    else
    {
        exp = exp ? cvt.apply(context_, exp) : nullptr;
        setExplanation(mkReturn(location_, exp));
    }
}
