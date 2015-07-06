#include <StdInc.h>
#include "Generic.h"
#include <Helpers/DeclsHelpers.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;

Generic::Generic(int nodeKind, DynNode* origNode, DynNodeVector genericParams, DynNode* ifClause, AccessType accessType)
    : DynNode(nodeKind, origNode->location(), { mkInstantiationsSet(origNode, move(genericParams), ifClause) }, { origNode })
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

const DynNodeVector& Generic::genericParams() const
{
    return children_[0]->as<InstantiationsSet>()->parameters();
}
