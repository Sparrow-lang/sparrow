#include <StdInc.h>
#include "GlobalDestructAction.h"
#include "FeatherNodeCommonsCpp.h"
#include "FeatherNodes.h"

#include <Util/TypeTraits.h>


using namespace Feather;

GlobalDestructAction::GlobalDestructAction(const Location& loc, DynNode* action)
    : DynNode(classNodeKind(), loc, {action})
{
}

DynNode* GlobalDestructAction::destructAction() const
{
    return data_.children[0];
}

void GlobalDestructAction::dump(ostream& os) const
{
    os << "globalDestructAction(" << data_.children[0] << ")";
}

void GlobalDestructAction::doSemanticCheck()
{
    data_.children[0]->semanticCheck();
    data_.type = getVoidType(data_.context->evalMode());

    // We never CT evaluate global destruct actions
    if ( isCt(data_.children[0]) )
    {
        setExplanation(mkNop(data_.location));
    }
}
