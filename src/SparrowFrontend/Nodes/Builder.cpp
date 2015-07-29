#include <StdInc.h>
#include "Builder.h"
#include <NodeCommonsCpp.h>

#include <Feather/Nodes/FeatherNodes.h>


using namespace SprFrontend;


vector<string>* SprFrontend::buildStringList(vector<string>* prevList, string element)
{
    if ( !prevList )
        prevList = new vector<string>;

    prevList->emplace_back(move(element));
    return prevList;
}

DynNode* SprFrontend::buildVariables(const Location& loc, const vector<string>& names, DynNode* typeNode, DynNode* init, NodeList* mods, AccessType accessType)
{
    NodeVector nodes;
    nodes.reserve(names.size());
    for ( const string& name: names )
    {
        nodes.push_back(mkModifiers(loc, mkSprVariable(loc, name, typeNode, init, accessType), mods)->node());
    }
    return (DynNode*) Feather::mkNodeList(loc, nodes, true);
}

NodeList* SprFrontend::buildParameters(const Location& loc, const vector<string>& names, DynNode* typeNode, DynNode* init, NodeList* mods)
{
    NodeVector nodes;
    nodes.reserve(names.size());
    for ( const string& name: names )
    {
        nodes.push_back(mkModifiers(loc, mkSprParameter(loc, name, typeNode, init), mods)->node());
    }
    return (NodeList*) Feather::mkNodeList(loc, nodes, true);
}

NodeList* SprFrontend::buildAutoParameter(const Location& loc, const string& name, NodeList* mods)
{
    DynNode* typeNode = mkIdentifier(loc, "AnyType");
    Node* param = mkModifiers(loc, mkSprParameter(loc, name, typeNode, nullptr), mods)->node();
    return (NodeList*) Feather::mkNodeList(loc, {param}, true);
}

