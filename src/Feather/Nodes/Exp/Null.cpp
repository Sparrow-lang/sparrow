#include <StdInc.h>
#include "Null.h"
#include <Feather/FeatherNodeCommonsCpp.h>
#include <Util/TypeTraits.h>


Null::Null(const Location& loc, Node* typeNode)
    : Node(loc, {typeNode})
{
}

void Null::dump(ostream& os) const
{
    os << "null(" << type() << ")";
}

void Null::doSemanticCheck()
{
    ASSERT(children_.size() == 1);
    Node* typeNode = children_[0];
    typeNode->computeType();

    // Make sure that the type is a reference
    TypeRef t = typeNode->type();
    if ( !t->hasStorage )
        REP_ERROR(location_, "Null node should have a type with storage (cur type: %1%") % t;
    if ( t->numReferences == 0 )
        REP_ERROR(location_, "Null node should have a reference type (cur type: %1%)") % t;

    type_ = adjustMode(t, context_, location_);
}
