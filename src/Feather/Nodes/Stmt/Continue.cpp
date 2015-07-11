#include <StdInc.h>
#include "Continue.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Util/Context.h>


Continue::Continue(const Location& location)
    : DynNode(classNodeKind(), location)
{
    setProperty("loop", (DynNode*) nullptr);
}

DynNode* Continue::loop() const
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
    DynNode* loop = getParentLoop(data_->context);
    if ( !loop )
        REP_ERROR(data_->location, "Continue found outside any loop");
    setProperty("loop", loop);

    // The resulting type is Void
    data_->type = getVoidType(data_->context->evalMode());
}
