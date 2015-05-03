#pragma once

#include <NodeCommonsH.h>

#include <SparrowFrontend/Nodes/SparrowNodes.h>

#include "Decls/AccessType.h"

namespace SprFrontend
{
    Node* addModifiers(NodeList* mods, Node* main);

    vector<string>* buildStringList(vector<string>* prevList, string element);

    Node* buildVariables(const Location& loc, const vector<string>& names, Node* typeNode, Node* init, NodeList* mods, AccessType accessType);
    NodeList* buildParameters(const Location& loc, const vector<string>& names, Node* typeNode, Node* init, NodeList* mods);
    NodeList* buildAutoParameter(const Location& loc, const string& name, NodeList* mods);
}
