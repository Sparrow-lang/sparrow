#include <StdInc.h>
#include "LocalSpace.h"
#include "FeatherNodeCommonsCpp.h"

#include <Util/TypeTraits.h>


LocalSpace::LocalSpace(const Location& location, NodeVector children)
    : Node(location, move(children))
{
}

void LocalSpace::addChild(Node* p)
{
    children_.push_back(p);
}

void LocalSpace::insertChildInFront(Node* p)
{
    children_.insert(children_.begin(), p);
}

void LocalSpace::dump(ostream& os) const
{
    os << "localSpace(";
    for ( Node* p: children_ )
    {
        os << endl << p;
    }
    os << ")";
}

void LocalSpace::doSetContextForChildren()
{
    childrenContext_ = context_->createChildContext(this);
    Node::doSetContextForChildren();
}

void LocalSpace::doComputeType()
{
    type_ = Type::fromBasicType(getVoidType(context_->evalMode()));
}

void LocalSpace::doSemanticCheck()
{
    // Compute type first
    doComputeType();

    // Semantic check each of the children
    for ( Node* p: children_ )
    {
        try
        {
            p->semanticCheck();
        }
        catch(...)
        {
            // Don't pass errors upwards
        }
    }
    checkEvalMode(this);
}
