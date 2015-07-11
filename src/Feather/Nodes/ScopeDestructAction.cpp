#include <StdInc.h>
#include "ScopeDestructAction.h"
#include "FeatherNodeCommonsCpp.h"

using namespace Feather;

ScopeDestructAction::ScopeDestructAction(const Location& loc, DynNode* action)
    : DynNode(classNodeKind(), loc, {action})
{
}

DynNode* ScopeDestructAction::destructAction() const
{
    return data_->children[0];
}

void ScopeDestructAction::dump(ostream& os) const
{
    os << "scopeDestructAction(" << data_->children[0] << ")";
}

void ScopeDestructAction::doSemanticCheck()
{
    data_->children[0]->semanticCheck();
    data_->type = getVoidType(data_->context->evalMode());
}
