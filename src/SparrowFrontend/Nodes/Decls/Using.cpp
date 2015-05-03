#include <StdInc.h>
#include "Using.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>

#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Nest;

Using::Using(const Location& loc, string alias, Node* usingNode, AccessType accessType)
    : Node(loc, {usingNode})
{
    if ( !alias.empty() )
        Feather::setName(this, move(alias));
    setAccessType(this, accessType);
}

Node* Using::source() const
{
    ASSERT(children_.size() == 1);
    return children_[0];
}

void Using::doSetContextForChildren()
{
    if ( Feather::hasName(this) )
        Feather::addToSymTab(this);

    Node::doSetContextForChildren();
}

void Using::doComputeType()
{
    ASSERT(children_.size() == 1);
    Node* usingNode = children_[0];
    const string* alias = getPropertyString("name");

    // Compile the using name
    usingNode->semanticCheck();
    
    if ( !alias || alias->empty() )
    {
        // Make sure that this node refers to one or more declaration
        Node* baseExp;
        NodeVector decls = getDeclsFromNode(usingNode, baseExp);
        if ( decls.empty() )
            REP_ERROR(usingNode->location(), "Invalid using name - no declarations can be found");

        // Add references in the current symbol tab
        for ( Node* decl: decls )
        {
            context_->currentSymTab()->enter(Feather::getName(decl), decl);
        }
    }
    else
    {
        // We added this node to the current sym tab, as we set shouldAddToSymTab_ to true
    }

    setExplanation(Feather::mkNop(location_));
}

void Using::doSemanticCheck()
{
    computeType();
}
