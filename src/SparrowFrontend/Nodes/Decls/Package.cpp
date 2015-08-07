#include <StdInc.h>
#include "Package.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>
#include <Feather/Util/Decl.h>
#include <Feather/FeatherTypes.h>

using namespace SprFrontend;
using namespace Nest;

Package::Package(const Location& loc, string name, Node* children, AccessType accessType)
    : DynNode(classNodeKind(), loc, { (DynNode*) children })
{
    Feather::setName(&data_, move(name));
    setAccessType(node(), accessType);
}

void Package::doSetContextForChildren()
{
    // TODO (now): Try to remove this
    Feather::addToSymTab(&data_);

    // If we don't have a children context, create one
    if ( !data_.childrenContext )
        data_.childrenContext = data_.context->createChildContext(node());

    // Set the context for all the children
    Nest::setContext(data_.children[0], data_.childrenContext);
}

void Package::doComputeType()
{
    // Compute the type for the children
    Nest::computeType(data_.children[0]);
    data_.explanation = data_.children[0];
    checkForAllowedNamespaceChildren(data_.children[0]);

    data_.type = Feather::getVoidType(modeCt);
}

void Package::doSemanticCheck()
{
    computeType();
    Nest::semanticCheck(data_.children[0]);
    data_.explanation = data_.children[0];
}
