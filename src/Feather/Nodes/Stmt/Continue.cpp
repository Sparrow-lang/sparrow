#include <StdInc.h>
#include "Continue.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Util/Context.h>


Continue::Continue(const Location& location)
    : Node(classNodeKind(), location)
{
    setProperty("loop", (Node*) nullptr);
}

Node* Continue::loop() const
{
    return getCheckPropertyNode("loop");
}

void Continue::dump(ostream& os) const
{
    os << "continue";
}

void Continue::doSemanticCheck()
{
    // Get the outer-most loop from the context
    Node* loop = getParentLoop(context_);
    if ( !loop )
        REP_ERROR(location_, "Continue found outside any loop");
    setProperty("loop", loop);

    // The resulting type is Void
    type_ = getVoidType(context_->evalMode());
}
