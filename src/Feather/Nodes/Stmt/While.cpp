#include <StdInc.h>
#include "While.h"
#include <Feather/FeatherNodeCommonsCpp.h>
#include <Nodes/FeatherNodes.h>
#include <Nodes/NodeList.h>

#include <Util/TypeTraits.h>
#include <Util/Ct.h>
#include <Util/Decl.h>


While::While(const Location& location, DynNode* condition, DynNode* body, DynNode* step, bool isCt)
    : DynNode(classNodeKind(), location, {condition, step, body})
{
    if ( isCt )
        setEvalMode(node(), modeCt);
}

DynNode* While::condition() const
{
    return (DynNode*) data_.children[0];
}

DynNode* While::body() const
{
    return (DynNode*) data_.children[2];
}

DynNode* While::step() const
{
    return (DynNode*) data_.children[1];
}


void While::dump(ostream& os) const
{
    os << "while(" << data_.children[0] << ", " << data_.children[1] << ", " << data_.children[2] << ")";
}

void While::doSetContextForChildren()
{
    data_.childrenContext = data_.context->createChildContext(node());
    CompilationContext* condContext = nodeEvalMode(node()) == modeCt ? new CompilationContext(data_.context, modeCt) : data_.childrenContext;

    Nest::setContext(data_.children[0], condContext); // condition
    if ( data_.children[1] )
        Nest::setContext(data_.children[1], condContext); // step
    if ( data_.children[2] )
        Nest::setContext(data_.children[2], data_.childrenContext); // body
}

void While::doSemanticCheck()
{
    DynNode* condition = (DynNode*) data_.children[0];
    DynNode* step = (DynNode*) data_.children[1];
    DynNode* body = (DynNode*) data_.children[2];
    
    // Semantic check the condition
    condition->semanticCheck();
    if ( step )
        step->computeType();

    // Check that the type of the condition is 'Testable'
    if ( !isTestable(condition->node()) )
        REP_ERROR(condition->location(), "The condition of the while is not Testable");

    if ( nodeEvalMode(node()) == modeCt )
    {
        if ( !isCt(condition->node()) )
            REP_ERROR(condition->location(), "The condition of the ct while should be available at compile-time (%1%)") % condition->type();
        if ( step && !isCt(step->node()) )
            REP_ERROR(step->location(), "The step of the ct while should be available at compile-time (%1%)") % step->type();

        // Create a node-list that will be our explanation
        DynNodeVector result;

        // Do the while
        while ( true )
        {
            // CT-evaluate the condition; if the condition evaluates to false, exit the while
            if ( !getBoolCtValue(theCompiler().ctEval(condition->node())) )
                break;

            // Put (a copy of) the body in the resulting node-list
            if ( body )
            {
                DynNode* curBody = body->clone();
                curBody->setContext(data_.context);
                curBody->semanticCheck();
                result.push_back(curBody);
            }

            // If we have a step, make sure to evaluate it
            if ( step )
            {
                theCompiler().ctEval(step->node());    // We don't need the actual result
            }

            // Unfortunately, we don't treat 'break' and 'continue' instructions inside the ct while instructions
        }
        result.push_back(mkNop(data_.location)); // Make sure our resulting type is Void

        // Set the explanation and exit
        setExplanation(mkNodeList(data_.location, result));
        return;
    }
    // Semantic check the body and the step
    if ( body )
        body->semanticCheck();
    if ( step )
        step->semanticCheck();

    // The resulting type is Void
    data_.type = getVoidType(data_.context->evalMode());
}
