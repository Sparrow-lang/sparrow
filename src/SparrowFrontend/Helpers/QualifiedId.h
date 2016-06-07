#pragma once

#include <NodeCommonsH.h>

#include <string>
#include <vector>

namespace SprFrontend
{
    /// Interprets a qualified id node; if the given node is a star expression, the last element on the result will be the empty string
    void interpretQualifiedId(Node* n, vector<pair<string, Node*>>& res);
}
