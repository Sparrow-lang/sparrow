#include <StdInc.h>
#include "Using.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>

#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Nest;

Using::Using(const Location& loc, string alias, DynNode* usingNode, AccessType accessType)
    : DynNode(classNodeKind(), loc, {usingNode})
{
    if ( !alias.empty() )
        Feather::setName(this, move(alias));
    setAccessType(this, accessType);
}

DynNode* Using::source() const
{
    ASSERT(data_.children.size() == 1);
    return (DynNode*) data_.children[0];
}

void Using::doSetContextForChildren()
{
    if ( Feather::hasName(this) )
        Feather::addToSymTab(this);

    DynNode::doSetContextForChildren();
}

void Using::doComputeType()
{
    ASSERT(data_.children.size() == 1);
    DynNode* usingNode = (DynNode*) data_.children[0];
    const string* alias = getPropertyString("name");

    // Compile the using name
    usingNode->semanticCheck();
    
    if ( !alias || alias->empty() )
    {
        // Make sure that this node refers to one or more declaration
        DynNode* baseExp;
        DynNodeVector decls = getDeclsFromNode(usingNode, baseExp);
        if ( decls.empty() )
            REP_ERROR(usingNode->location(), "Invalid using name - no declarations can be found");

        // Add references in the current symbol tab
        for ( DynNode* decl: decls )
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
    computeType();
}
