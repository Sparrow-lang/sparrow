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

Node* SprFrontend::buildVariables(const Location& loc, const vector<string>& names, Node* typeNode, Node* init, Node* mods, AccessType accessType)
{
    NodeVector nodes;
    nodes.reserve(names.size());
    for ( const string& name: names )
    {
        nodes.push_back(mkModifiers(loc, mkSprVariable(loc, name, typeNode, init, accessType), mods));
    }
    return Feather::mkNodeList(loc, nodes, true);
}

Node* SprFrontend::buildParameters(const Location& loc, const vector<string>& names, Node* typeNode, Node* init, Node* mods)
{
    NodeVector nodes;
    nodes.reserve(names.size());
    for ( const string& name: names )
    {
        nodes.push_back(mkModifiers(loc, mkSprParameter(loc, name, typeNode, init), mods));
    }
    return Feather::mkNodeList(loc, nodes, true);
}

Node* SprFrontend::buildAutoParameter(const Location& loc, const string& name, Node* mods)
{
    Node* typeNode = mkIdentifier(loc, "AnyType");
    Node* param = mkModifiers(loc, mkSprParameter(loc, name, typeNode, nullptr), mods);
    return Feather::mkNodeList(loc, {param}, true);
}

