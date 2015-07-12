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
    return (DynNode*) data_.children[0];
}

void ScopeDestructAction::dump(ostream& os) const
{
    os << "scopeDestructAction(" << data_.children[0] << ")";
}

void ScopeDestructAction::doSemanticCheck()
{
    Nest::semanticCheck(data_.children[0]);
    data_.type = getVoidType(data_.context->evalMode());
}
