#include <StdInc.h>
#include "If.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Nodes/FeatherNodes.h>
#include <Util/TypeTraits.h>
#include <Util/Ct.h>
#include <Util/Decl.h>


If::If(const Location& location, DynNode* condition, DynNode* thenClause, DynNode* elseClause, bool isCt)
    : DynNode(classNodeKind(), location, {condition, thenClause, elseClause})
{
    if ( isCt )
        setEvalMode(this, modeCt);
}

DynNode* If::condition() const
{
    return data_->children[0];
}

DynNode* If::thenClause() const
{
    return data_->children[1];
}

DynNode* If::elseClause() const
{
    return data_->children[2];
}

void If::dump(ostream& os) const
{
    os << "if(" << data_->children[0] << ", " << data_->children[1] << ", " << data_->children[2] << ")";
}

void If::doSetContextForChildren()
{
    data_->childrenContext = data_->context->createChildContext(this);
    
    data_->children[0]->setContext(data_->childrenContext);
    if ( data_->children[1] )
        data_->children[1]->setContext(data_->childrenContext);
    if ( data_->children[2] )
        data_->children[2]->setContext(data_->childrenContext);
}

void If::doSemanticCheck()
{
    DynNode* condition = data_->children[0];
    DynNode* thenClause = data_->children[1];
    DynNode* elseClause = data_->children[2];
    
    // The resulting type is Void
    data_->type = getVoidType(data_->context->evalMode());

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
    data_->children[0] = condition;
    // TODO (if): Remove this dereference from here

    if ( nodeEvalMode(this) == modeCt )
    {
        if ( !isCt(condition) )
            REP_ERROR(condition->location(), "The condition of the ct if should be available at compile-time (%1%)") % condition->type();

        // Get the CT value from the condition, and select an active branch
        DynNode* c = theCompiler().ctEval(condition);
        DynNode* selectedBranch = getBoolCtValue(c) ? thenClause : elseClause;

        // Expand only the selected branch
        if ( selectedBranch )
            setExplanation(selectedBranch);
        else
            setExplanation(mkNop(data_->location));
        return;
    }

    // Semantic check the clauses
    if ( thenClause )
        thenClause->semanticCheck();
    if ( elseClause )
        elseClause->semanticCheck();
}
