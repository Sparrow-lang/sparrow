#include <StdInc.h>
#include "Using.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>

#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Nest;

Using::Using(const Location& loc, string alias, Node* usingNode, AccessType accessType)
    : DynNode(classNodeKind(), loc, {usingNode})
{
    if ( !alias.empty() )
        Feather::setName(node(), move(alias));
    setAccessType(node(), accessType);
}

Node* Using::source() const
{
    ASSERT(data_.children.size() == 1);
    return data_.children[0];
}

void Using::doSetContextForChildren()
{
    if ( Feather::hasName(node()) )
        Feather::addToSymTab(node());

    DynNode::doSetContextForChildren();
}

void Using::doComputeType()
{
    ASSERT(data_.children.size() == 1);
    Node* usingNode = data_.children[0];
    const string* alias = getPropertyString("name");

    // Compile the using name
    Nest::semanticCheck(usingNode);
    
    if ( !alias || alias->empty() )
    {
        // Make sure that this node refers to one or more declaration
        Node* baseExp;
        NodeVector decls = getDeclsFromNode(usingNode, baseExp);
        if ( decls.empty() )
            REP_ERROR(usingNode->location, "Invalid using name - no declarations can be found");

        // Add references in the current symbol tab
        for ( Node* decl: decls )
        {
            data_.context->currentSymTab()->enter(Feather::getName(decl), decl);
        }
    }
    else
    {
        // We added this node to the current sym tab, as we set shouldAddToSymTab_ to true
    }

    setExplanation(Feather::mkNop(data_.location));
}

void Using::doSemanticCheck()
{
    Nest::computeType(node());
}
