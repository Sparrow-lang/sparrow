#include <StdInc.h>
#include "QualifiedId.h"

#include <Feather/Util/Decl.h>

using namespace Feather;

void SprFrontend::interpretQualifiedId(DynNode* n, vector<string>& res)
{
    ASSERT(n);
    if ( n->nodeKind() == nkSparrowExpIdentifier )
    {
        res.emplace_back(getName(n->node()));
    }
    else if ( n->nodeKind() == nkSparrowExpCompoundExp )
    {
        DynNode* base = n->children()[0];
        interpretQualifiedId(base, res);
        res.emplace_back(getName(n->node()));
    }
    else if ( n->nodeKind() == nkSparrowExpStarExp )
    {
        DynNode* base = n->children()[0];
        interpretQualifiedId(base, res);
        res.emplace_back(string());
    }
    else
        REP_INTERNAL(n->location(), "Don't know how to interpret node of type %1% in qualified id") % n->nodeKindName();
}
