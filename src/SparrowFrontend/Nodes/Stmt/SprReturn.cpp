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
    ASSERT(data_.children.size() == 1);
    DynNode* exp = (DynNode*) data_.children[0];

    // Get the parent function of this return
    Function* parentFun = getParentFun(data_.context);
    if ( !parentFun )
        REP_ERROR(data_.location, "Return found outside any function");

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
            setExplanation((DynNode*) mkNodeList(data_.location, { exp->node(), mkReturn(data_.location) }));
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
            REP_ERROR(data_.location, "You must return something in a function that has non-Void result type");
    }

    // Build the explanation of this node
    if ( resultParam )
    {
        // Create a ctor to construct the result parameter with the expression received
        const Location& l = resultParam->location();
        DynNode* thisArg = (DynNode*) mkMemLoad(l, mkVarRef(l, resultParam->node()));
        thisArg->setContext(data_.context);
        DynNode* action = createCtorCall(l, data_.context, thisArg, exp);
        if ( !action )
            REP_ERROR(exp->location(), "Cannot construct return type object %1% from %2%") % exp->type() % resType;

        if ( action )
        {
            setExplanation((DynNode*) mkNodeList(data_.location, { action->node(), mkReturn(data_.location, nullptr)}));
        }
    }
    else
    {
        exp = exp ? cvt.apply(data_.context, exp) : nullptr;
        setExplanation((DynNode*) mkReturn(data_.location, exp->node()));
    }
}
