#include <StdInc.h>
#include "LocalSpace.h"
#include "FeatherNodeCommonsCpp.h"

#include <Util/TypeTraits.h>


LocalSpace::LocalSpace(const Location& location, DynNodeVector children)
    : DynNode(classNodeKind(), location, move(children))
{
}

void LocalSpace::addChild(DynNode* p)
{
    children_.push_back(p);
}

void LocalSpace::insertChildInFront(DynNode* p)
{
    children_.insert(children_.begin(), p);
}

void LocalSpace::dump(ostream& os) const
{
    os << "localSpace(";
    for ( DynNode* p: children_ )
    {
        os << endl << p;
    }
    os << ")";
}

void LocalSpace::doSetContextForChildren()
{
    childrenContext_ = context_->createChildContext(this);
    DynNode::doSetContextForChildren();
}

void LocalSpace::doComputeType()
{
    type_ = getVoidType(context_->evalMode());
}

void LocalSpace::doSemanticCheck()
{
    // Compute type first
    doComputeType();

    // Semantic check each of the children
    for ( DynNode* p: children_ )
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
