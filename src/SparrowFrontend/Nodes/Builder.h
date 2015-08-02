#pragma once

#include <NodeCommonsH.h>

#include <SparrowFrontend/Nodes/SparrowNodes.h>

#include "Decls/AccessType.h"

namespace SprFrontend
{
    Node* addModifiers(Node* mods, Node* main);

    vector<string>* buildStringList(vector<string>* prevList, string element);

    Node* buildVariables(const Location& loc, const vector<string>& names, Node* typeNode, Node* init, Node* mods, AccessType accessType);
    Node* buildParameters(const Location& loc, const vector<string>& names, Node* typeNode, Node* init, Node* mods);
    Node* buildAutoParameter(const Location& loc, const string& name, Node* mods);
}
