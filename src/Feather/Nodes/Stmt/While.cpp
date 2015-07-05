#include <StdInc.h>
#include "While.h"
#include <Feather/FeatherNodeCommonsCpp.h>
#include <Nodes/FeatherNodes.h>
#include <Nodes/NodeList.h>

#include <Util/TypeTraits.h>
#include <Util/Ct.h>
#include <Util/Decl.h>


While::While(const Location& location, Node* condition, Node* body, Node* step, bool isCt)
    : Node(location, {condition, step, body})
{
    if ( isCt )
        setEvalMode(this, modeCt);
}

Node* While::condition() const
{
    return children_[0];
}

Node* While::body() const
{
    return children_[2];
}

Node* While::step() const
{
    return children_[1];
}


void While::dump(ostream& os) const
{
    os << "while(" << children_[0] << ", " << children_[1] << ", " << children_[2] << ")";
}

void While::doSetContextForChildren()
{
    childrenContext_ = context_->createChildContext(this);
    CompilationContext* condContext = nodeEvalMode(this) == modeCt ? new CompilationContext(context_, modeCt) : childrenContext_;

    children_[0]->setContext(condContext); // condition
    if ( children_[1] )
        children_[1]->setContext(condContext); // step
    if ( children_[2] )
        children_[2]->setContext(childrenContext_); // body
}

void While::doSemanticCheck()
{
    Node* condition = children_[0];
    Node* step = children_[1];
    Node* body = children_[2];
    
    // Semantic check the condition
    condition->semanticCheck();
    if ( step )
        step->computeType();

    // Check that the type of the condition is 'Testable'
    if ( !isTestable(condition) )
        REP_ERROR(condition->location(), "The condition of the while is not Testable");

    if ( nodeEvalMode(this) == modeCt )
    {
        if ( !isCt(condition) )
            REP_ERROR(condition->location(), "The condition of the ct while should be available at compile-time (%1%)") % condition->type();
        if ( step && !isCt(step) )
            REP_ERROR(step->location(), "The step of the ct while should be available at compile-time (%1%)") % step->type();

        // Create a node-list that will be our explanation
        NodeVector result;

        // Do the while
        while ( true )
        {
            // CT-evaluate the condition; if the condition evaluates to false, exit the while
            if ( !getBoolCtValue(theCompiler().ctEval(condition)) )
                break;

            // Put (a copy of) the body in the resulting node-list
            if ( body )
            {
                Node* curBody = body->clone();
                curBody->setContext(context_);
                curBody->semanticCheck();
                result.push_back(curBody);
            }

            // If we have a step, make sure to evaluate it
            if ( step )
            {
                theCompiler().ctEval(step);    // We don't need the actual result
            }

            // Unfortunately, we don't treat 'break' and 'continue' instructions inside the ct while instructions
        }
        result.push_back(mkNop(location_)); // Make sure our resulting type is Void

        // Set the explanation and exit
        setExplanation(mkNodeList(location_, result));
        return;
    }
    // Semantic check the body and the step
    if ( body )
        body->semanticCheck();
    if ( step )
        step->semanticCheck();

    // The resulting type is Void
    type_ = getVoidType(context_->evalMode());
}
