#pragma once

#include <NodeCommonsH.h>

#include <SparrowFrontend/Nodes/SparrowNodes.h>

#include "Decls/AccessType.h"

namespace SprFrontend
{
    DynNode* addModifiers(NodeList* mods, DynNode* main);

    vector<string>* buildStringList(vector<string>* prevList, string element);

    DynNode* buildVariables(const Location& loc, const vector<string>& names, DynNode* typeNode, DynNode* init, NodeList* mods, AccessType accessType);
    NodeList* buildParameters(const Location& loc, const vector<string>& names, DynNode* typeNode, DynNode* init, NodeList* mods);
    NodeList* buildAutoParameter(const Location& loc, const string& name, NodeList* mods);
}
