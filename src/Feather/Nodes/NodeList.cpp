#include <StdInc.h>
#include "NodeList.h"
#include <Util/TypeTraits.h>
#include <Feather/FeatherTypes.h>

using namespace Feather;

namespace
{
    const char* propResultVoid = "nodeList.resultVoid";
}

NodeList::NodeList(const Location& loc, DynNodeVector children, bool resultVoid)
    : DynNode(classNodeKind(), loc, move(children))
{
    if ( resultVoid )
        setProperty(propResultVoid, 1);
}

void NodeList::addChild(DynNode* p)
{
    data_.children.push_back(p->node());
}

void NodeList::dump(ostream& os) const
{
    os << "nodeList(";
    for ( Node* p: data_.children )
    {
        os << endl << p;
    }
    os << ")";
}

void NodeList::doComputeType()
{
    // Compute the type for all the children
    for ( Node* p: data_.children )
    {
        if ( !p )
            continue;

        Nest::computeType(p);
    }

    // Get the type of the last node
    data_.type = ( hasProperty(propResultVoid) || data_.children.empty() || !data_.children.back()->type ) ? getVoidType(data_.context->evalMode()) : data_.children.back()->type;
    data_.type = adjustMode(data_.type, data_.context, data_.location);
    checkEvalMode(node());
}

void NodeList::doSemanticCheck()
{
    // Semantic check each of the children
    bool hasNonCtChildren = false;
    for ( Node* p: data_.children )
    {
        if ( !p )
            continue;

        Nest::semanticCheck(p);
        hasNonCtChildren = hasNonCtChildren || !isCt(p);
    }

    // Make sure the type is computed
    if ( !data_.type )
    {
        // Get the type of the last node
        data_.type = ( hasProperty(propResultVoid) || data_.children.empty() || !data_.children.back()->type ) ? getVoidType(data_.context->evalMode()) : data_.children.back()->type;
        data_.type = adjustMode(data_.type, data_.context, data_.location);
        checkEvalMode(node());
    }
}
