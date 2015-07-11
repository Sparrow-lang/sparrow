#include <StdInc.h>
#include "ChangeMode.h"
#include "FeatherNodeCommonsCpp.h"


ChangeMode::ChangeMode(const Location& loc, EvalMode mode, DynNode* child)
    : DynNode(classNodeKind(), loc)
{
    if ( child )
        data_->children.push_back(child);
    setProperty(propEvalMode, (int) mode);
}

DynNode* ChangeMode::child() const
{
    return data_->children[0];
}

void ChangeMode::setChild(DynNode* child)
{
    ASSERT(child);
    data_->children.resize(1);
    data_->children[0] = child;

    if ( data_->childrenContext )
        child->setContext(data_->childrenContext);
}

EvalMode ChangeMode::evalMode() const
{
    EvalMode curMode = (EvalMode) getCheckPropertyInt(propEvalMode);
    return curMode != modeUnspecified ? curMode : data_->context->evalMode();
}

void ChangeMode::dump(ostream& os) const
{
    os << "changeMode(" << data_->children[0] << ", " << evalMode() << ")";
}

void ChangeMode::doSetContextForChildren()
{
    data_->childrenContext = new CompilationContext(data_->context, evalMode());
    DynNode::doSetContextForChildren();
}

void ChangeMode::doSemanticCheck()
{
    // Make sure we are allowed to change the mode
    EvalMode baseMode = data_->context->evalMode();
    EvalMode newMode = evalMode();
    if ( newMode == modeUnspecified )
        REP_INTERNAL(data_->location, "Cannot change the mode to Unspecified");
    if ( newMode == modeRt && baseMode != modeRt )
        REP_ERROR(data_->location, "Cannot change mode to RT in a non-RT context (%1%)") % baseMode;
    if ( newMode == modeRtCt && baseMode != modeRtCt )
        REP_ERROR(data_->location, "Cannot change mode to RTCT in a non-RTCT context (%1%)") % baseMode;

    if ( !data_->children[0] )
        REP_INTERNAL(data_->location, "No node specified as child to a ChangeMode node");

    data_->children[0]->semanticCheck();
    setExplanation(data_->children[0]);
}
