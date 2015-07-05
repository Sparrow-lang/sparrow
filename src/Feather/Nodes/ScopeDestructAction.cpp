#include <StdInc.h>
#include "ScopeDestructAction.h"
#include "FeatherNodeCommonsCpp.h"

using namespace Feather;

ScopeDestructAction::ScopeDestructAction(const Location& loc, Node* action)
    : Node(loc, {action})
{
}

Node* ScopeDestructAction::destructAction() const
{
    return children_[0];
}

void ScopeDestructAction::dump(ostream& os) const
{
    os << "scopeDestructAction(" << children_[0] << ")";
}

void ScopeDestructAction::doSemanticCheck()
{
    children_[0]->semanticCheck();
    type_ = getVoidType(context_->evalMode());
}
