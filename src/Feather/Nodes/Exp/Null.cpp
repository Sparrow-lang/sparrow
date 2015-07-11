#include <StdInc.h>
#include "Null.h"
#include <Feather/FeatherNodeCommonsCpp.h>
#include <Util/TypeTraits.h>


Null::Null(const Location& loc, DynNode* typeNode)
    : DynNode(classNodeKind(), loc, {typeNode})
{
}

void Null::dump(ostream& os) const
{
    os << "null(" << type() << ")";
}

void Null::doSemanticCheck()
{
    ASSERT(data_->children.size() == 1);
    DynNode* typeNode = data_->children[0];
    typeNode->computeType();

    // Make sure that the type is a reference
    TypeRef t = typeNode->type();
    if ( !t->hasStorage )
        REP_ERROR(data_->location, "Null node should have a type with storage (cur type: %1%") % t;
    if ( t->numReferences == 0 )
        REP_ERROR(data_->location, "Null node should have a reference type (cur type: %1%)") % t;

    data_->type = adjustMode(t, data_->context, data_->location);
}
