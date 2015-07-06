#include <StdInc.h>
#include "Break.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Util/Context.h>


Break::Break(const Location& location)
    : DynNode(classNodeKind(), location)
{
    setProperty("loop", (DynNode*) nullptr);
}

DynNode* Break::loop() const
{
    return getCheckPropertyNode("loop");
}

void Break::dump(ostream& os) const
{
    os << "break";
}

void Break::doSemanticCheck()
{
    // Get the outer-most loop from the context
    DynNode* loop = getParentLoop(context_);
    if ( !loop )
        REP_ERROR(location_, "Break found outside any loop");
    setProperty("loop", loop);

    // The resulting type is Void
    type_ = getVoidType(context_->evalMode());
}
