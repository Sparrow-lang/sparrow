#include <StdInc.h>
#include "GlobalConstructAction.h"
#include "FeatherNodeCommonsCpp.h"
#include "FeatherNodes.h"

#include <Util/TypeTraits.h>


GlobalConstructAction::GlobalConstructAction(const Location& loc, DynNode* action)
    : DynNode(classNodeKind(), loc, {action})
{
}

DynNode* GlobalConstructAction::constructAction() const
{
    return data_->children[0];
}

void GlobalConstructAction::dump(ostream& os) const
{
    os << "globalConstructAction(" << data_->children[0] << ")";
}

void GlobalConstructAction::doSemanticCheck()
{
    data_->children[0]->semanticCheck();
    data_->type = getVoidType(data_->context->evalMode());

    // For CT construct actions, evaluate them asap
    if ( isCt(data_->children[0]) )
    {
        theCompiler().ctEval(data_->children[0]);
        setExplanation(mkNop(data_->location));
    }
}

