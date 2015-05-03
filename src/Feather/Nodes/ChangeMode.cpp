#include <StdInc.h>
#include "ChangeMode.h"
#include "FeatherNodeCommonsCpp.h"


ChangeMode::ChangeMode(const Location& loc, EvalMode mode, Node* child)
    : Node(loc)
{
    if ( child )
        children_.push_back(child);
    setProperty(propEvalMode, (int) mode);
}

Node* ChangeMode::child() const
{
    return children_[0];
}

void ChangeMode::setChild(Node* child)
{
    ASSERT(child);
    children_.resize(1);
    children_[0] = child;

    if ( childrenContext_ )
        child->setContext(childrenContext_);
}

EvalMode ChangeMode::evalMode() const
{
    EvalMode curMode = (EvalMode) getCheckPropertyInt(propEvalMode);
    return curMode != modeUnspecified ? curMode : context_->evalMode();
}

void ChangeMode::dump(ostream& os) const
{
    os << "changeMode(" << children_[0] << ", " << evalMode() << ")";
}

void ChangeMode::doSetContextForChildren()
{
    childrenContext_ = new CompilationContext(context_, evalMode());
    Node::doSetContextForChildren();
}

void ChangeMode::doSemanticCheck()
{
    // Make sure we are allowed to change the mode
    EvalMode baseMode = context_->evalMode();
    EvalMode newMode = evalMode();
    if ( newMode == modeUnspecified )
        REP_INTERNAL(location_, "Cannot change the mode to Unspecified");
    if ( newMode == modeRt && baseMode != modeRt )
        REP_ERROR(location_, "Cannot change mode to RT in a non-RT context (%1%)") % baseMode;
    if ( newMode == modeRtCt && baseMode != modeRtCt )
        REP_ERROR(location_, "Cannot change mode to RTCT in a non-RTCT context (%1%)") % baseMode;

    if ( !children_[0] )
        REP_INTERNAL(location_, "No node specified as child to a ChangeMode node");

    children_[0]->semanticCheck();
    setExplanation(children_[0]);
}
