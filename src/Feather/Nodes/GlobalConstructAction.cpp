#include <StdInc.h>
#include "GlobalConstructAction.h"
#include "FeatherNodeCommonsCpp.h"
#include "FeatherNodes.h"

#include <Type/Void.h>
#include <Util/TypeTraits.h>


GlobalConstructAction::GlobalConstructAction(const Location& loc, Node* action)
    : Node(loc, {action})
{
}

Node* GlobalConstructAction::constructAction() const
{
    return children_[0];
}

void GlobalConstructAction::dump(ostream& os) const
{
    os << "globalConstructAction(" << children_[0] << ")";
}

void GlobalConstructAction::doSemanticCheck()
{
    children_[0]->semanticCheck();
    type_ = Void::get(context_->evalMode());

    // For CT construct actions, evaluate them asap
    if ( isCt(children_[0]) )
    {
        theCompiler().ctEval(children_[0]);
        setExplanation(mkNop(location_));
    }
}

