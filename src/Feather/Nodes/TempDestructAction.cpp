#include <StdInc.h>
#include "TempDestructAction.h"
#include "FeatherNodeCommonsCpp.h"

using namespace Feather;

TempDestructAction::TempDestructAction(const Location& loc, DynNode* action)
    : DynNode(classNodeKind(), loc, {action})
{
}

DynNode* TempDestructAction::destructAction() const
{
    return data_->children[0];
}

void TempDestructAction::dump(ostream& os) const
{
    os << "tempDestructAction(" << data_->children[0] << ")";
}

void TempDestructAction::doSemanticCheck()
{
    data_->children[0]->semanticCheck();
    data_->type = getVoidType(data_->context->evalMode());
}
