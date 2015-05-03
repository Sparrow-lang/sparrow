#include <StdInc.h>
#include "GlobalDestructAction.h"
#include "FeatherNodeCommonsCpp.h"
#include "FeatherNodes.h"

#include <Type/Void.h>
#include <Util/TypeTraits.h>


using namespace Feather;

GlobalDestructAction::GlobalDestructAction(const Location& loc, Node* action)
    : Node(loc, {action})
{
}

Node* GlobalDestructAction::destructAction() const
{
    return children_[0];
}

void GlobalDestructAction::dump(ostream& os) const
{
    os << "globalDestructAction(" << children_[0] << ")";
}

void GlobalDestructAction::doSemanticCheck()
{
    children_[0]->semanticCheck();
    type_ = Void::get(context_->evalMode());

    // We never CT evaluate global destruct actions
    if ( isCt(children_[0]) )
    {
        setExplanation(mkNop(location_));
    }
}
