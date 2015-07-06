#include <StdInc.h>
#include "Generic.h"
#include <Helpers/DeclsHelpers.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;

Generic::Generic(int nodeKind, Node* origNode, NodeVector genericParams, Node* ifClause, AccessType accessType)
    : Node(nodeKind, origNode->location(), { mkInstantiationsSet(origNode, move(genericParams), ifClause) }, { origNode })
{
    setName(this, getName(origNode));
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

const NodeVector& Generic::genericParams() const
{
    return children_[0]->as<InstantiationsSet>()->parameters();
}
