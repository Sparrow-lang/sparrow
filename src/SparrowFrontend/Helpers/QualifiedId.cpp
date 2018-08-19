#include <StdInc.h>
#include "QualifiedId.h"

#include "Feather/Utils/FeatherUtils.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"

void SprFrontend::interpretQualifiedId(Node* n, QidVec& res) {
    ASSERT(n);
    if (n->nodeKind == nkSparrowExpIdentifier) {
        res.emplace_back(make_pair(Feather_getName(n), n->location));
    } else if (n->nodeKind == nkSparrowExpCompoundExp) {
        Node* base = at(n->children, 0);
        interpretQualifiedId(base, res);
        res.emplace_back(make_pair(Feather_getName(n), n->location));
    } else if (n->nodeKind == nkSparrowExpStarExp) {
        Node* base = at(n->children, 0);
        interpretQualifiedId(base, res);
        res.emplace_back(make_pair(StringRef{}, n->location));
    } else
        REP_INTERNAL(n->location, "Don't know how to interpret node %1% in qualified id") % n;
}
