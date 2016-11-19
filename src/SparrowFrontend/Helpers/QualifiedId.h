#pragma once

#include <NodeCommonsH.h>

#include <vector>

namespace SprFrontend
{
    /// A QID: a list of names with the associated locations
    typedef vector<pair<StringRef, Location>> QidVec;

    /// Interprets a qualified id node; if the given node is a star expression, the last element on the result will be the empty string
    void interpretQualifiedId(Node* n, QidVec& res);
}
