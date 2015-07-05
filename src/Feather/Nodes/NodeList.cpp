#include <StdInc.h>
#include "NodeList.h"
#include <Util/TypeTraits.h>
#include <Feather/FeatherTypes.h>

using namespace Feather;

namespace
{
    const char* propResultVoid = "nodeList.resultVoid";
}

NodeList::NodeList(const Location& loc, NodeVector children, bool resultVoid)
    : Node(loc, move(children))
{
    if ( resultVoid )
        setProperty(propResultVoid, 1);
}

void NodeList::addChild(Node* p)
{
    children_.push_back(p);
}

void NodeList::dump(ostream& os) const
{
    os << "nodeList(";
    for ( Node* p: children_ )
    {
        os << endl << p;
    }
    os << ")";
}

void NodeList::doComputeType()
{
    // Compute the type for all the children
    for ( Node* p: children_ )
    {
        if ( !p )
            continue;
        
        p->computeType();
    }

    // Get the type of the last node
    type_ = ( hasProperty(propResultVoid) || children_.empty() || !children_.back()->type() ) ? getVoidType(context_->evalMode()) : children_.back()->type();
    type_ = adjustMode(type_, context_, location_);
    checkEvalMode(this);
}

void NodeList::doSemanticCheck()
{
    // Semantic check each of the children
    bool hasNonCtChildren = false;
    for ( Node* p: children_ )
    {
        if ( !p )
            continue;
        
        p->semanticCheck();
        hasNonCtChildren = hasNonCtChildren || !isCt(p);
    }

    // Make sure the type is computed
    if ( !type_ )
    {
        // Get the type of the last node
        type_ = ( hasProperty(propResultVoid) || children_.empty() || !children_.back()->type() ) ? getVoidType(context_->evalMode()) : children_.back()->type();
        type_ = adjustMode(type_, context_, location_);
        checkEvalMode(this);
    }
}
