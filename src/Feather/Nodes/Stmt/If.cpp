#include <StdInc.h>
#include "If.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Nodes/FeatherNodes.h>
#include <Util/TypeTraits.h>
#include <Util/Ct.h>
#include <Util/Decl.h>


If::If(const Location& location, Node* condition, Node* thenClause, Node* elseClause, bool isCt)
    : Node(classNodeKind(), location, {condition, thenClause, elseClause})
{
    if ( isCt )
        setEvalMode(this, modeCt);
}

Node* If::condition() const
{
    return children_[0];
}

Node* If::thenClause() const
{
    return children_[1];
}

Node* If::elseClause() const
{
    return children_[2];
}

void If::dump(ostream& os) const
{
    os << "if(" << children_[0] << ", " << children_[1] << ", " << children_[2] << ")";
}

void If::doSetContextForChildren()
{
    childrenContext_ = context_->createChildContext(this);
    
    children_[0]->setContext(childrenContext_);
    if ( children_[1] )
        children_[1]->setContext(childrenContext_);
    if ( children_[2] )
        children_[2]->setContext(childrenContext_);
}

void If::doSemanticCheck()
{
    Node* condition = children_[0];
    Node* thenClause = children_[1];
    Node* elseClause = children_[2];
    
    // The resulting type is Void
    type_ = getVoidType(context_->evalMode());

    // Semantic check the condition
    condition->semanticCheck();

    // Check that the type of the condition is 'Testable'
    if ( !isTestable(condition) )
        REP_ERROR(condition->location(), "The condition of the if is not Testable");

    // Dereference the condition as much as possible
    while ( condition->type() && condition->type()->numReferences > 0 )
    {
        condition = mkMemLoad(condition->location(), condition);
        condition->setContext(childrenContext());
        condition->semanticCheck();
    }
    children_[0] = condition;
    // TODO (if): Remove this dereference from here

    if ( nodeEvalMode(this) == modeCt )
    {
        if ( !isCt(condition) )
            REP_ERROR(condition->location(), "The condition of the ct if should be available at compile-time (%1%)") % condition->type();

        // Get the CT value from the condition, and select an active branch
        Node* c = theCompiler().ctEval(condition);
        Node* selectedBranch = getBoolCtValue(c) ? thenClause : elseClause;

        // Expand only the selected branch
        if ( selectedBranch )
            setExplanation(selectedBranch);
        else
            setExplanation(mkNop(location_));
        return;
    }

    // Semantic check the clauses
    if ( thenClause )
        thenClause->semanticCheck();
    if ( elseClause )
        elseClause->semanticCheck();
}
