#include <StdInc.h>
#include "Generic.h"
#include <Helpers/DeclsHelpers.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;

Generic::Generic(const Location& loc, const string& name, AccessType accessType)
    : Node(loc)
{
    setName(this, move(name));
    setAccessType(this, accessType);
}

Generic::~Generic()
{
}

void Generic::doSemanticCheck()
{
    explanation_ = mkNop(location_);
    explanation_->setContext(context_);
    explanation_->semanticCheck();
    type_ = explanation_->type();
}
