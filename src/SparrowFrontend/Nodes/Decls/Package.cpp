#include <StdInc.h>
#include "Package.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>
#include <Feather/Util/Decl.h>
#include <Feather/FeatherTypes.h>

using namespace SprFrontend;
using namespace Nest;

Package::Package(const Location& loc, string name, NodeList* children, AccessType accessType)
    : Node(loc, {children})
{
    Feather::setName(this, move(name));
    setAccessType(this, accessType);
}

void Package::doSetContextForChildren()
{
    // TODO (now): Try to remove this
    Feather::addToSymTab(this);

    // If we don't have a children context, create one
    if ( !childrenContext_ )
        childrenContext_ = context_->createChildContext(this);

    // Set the context for all the children
    children_[0]->setContext(childrenContext_);
}

void Package::doComputeType()
{
    // Compute the type for the children
    children_[0]->computeType();
    explanation_ = children_[0];
    checkForAllowedNamespaceChildren((NodeList*) children_[0]);

    type_ = Type::fromBasicType(Feather::getVoidType(modeCt));
}

void Package::doSemanticCheck()
{
    computeType();
    children_[0]->semanticCheck();
    explanation_ = children_[0];
}
