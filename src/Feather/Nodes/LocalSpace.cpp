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
    data_.children.push_back(p);
}

void LocalSpace::insertChildInFront(DynNode* p)
{
    data_.children.insert(data_.children.begin(), p);
}

void LocalSpace::dump(ostream& os) const
{
    os << "localSpace(";
    for ( DynNode* p: data_.children )
    {
        os << endl << p;
    }
    os << ")";
}

void LocalSpace::doSetContextForChildren()
{
    data_.childrenContext = data_.context->createChildContext(this);
    DynNode::doSetContextForChildren();
}

void LocalSpace::doComputeType()
{
    data_.type = getVoidType(data_.context->evalMode());
}

void LocalSpace::doSemanticCheck()
{
    // Compute type first
    doComputeType();

    // Semantic check each of the children
    for ( DynNode* p: data_.children )
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
