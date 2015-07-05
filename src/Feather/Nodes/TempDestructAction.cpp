#include <StdInc.h>
#include "TempDestructAction.h"
#include "FeatherNodeCommonsCpp.h"

using namespace Feather;

TempDestructAction::TempDestructAction(const Location& loc, Node* action)
    : Node(loc, {action})
{
}

Node* TempDestructAction::destructAction() const
{
    return children_[0];
}

void TempDestructAction::dump(ostream& os) const
{
    os << "tempDestructAction(" << children_[0] << ")";
}

void TempDestructAction::doSemanticCheck()
{
    children_[0]->semanticCheck();
    type_ = getVoidType(context_->evalMode());
}
