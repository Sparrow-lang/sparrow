#include <StdInc.h>
#include "QualifiedId.h"

#include <Feather/Util/Decl.h>
#include "Nest/Utils/NodeUtils.hpp"

using namespace Feather;

void SprFrontend::interpretQualifiedId(Node* n, vector<string>& res)
{
    ASSERT(n);
    if ( n->nodeKind == nkSparrowExpIdentifier )
    {
        res.emplace_back(toString(getName(n)));
    }
    else if ( n->nodeKind == nkSparrowExpCompoundExp )
    {
        Node* base = at(n->children, 0);
        interpretQualifiedId(base, res);
        res.emplace_back(toString(getName(n)));
    }
    else if ( n->nodeKind == nkSparrowExpStarExp )
    {
        Node* base = at(n->children, 0);
        interpretQualifiedId(base, res);
        res.emplace_back(string());
    }
    else
        REP_INTERNAL(n->location, "Don't know how to interpret node %1% in qualified id") % n;
}
