#include <StdInc.h>
#include "FunRef.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Nodes/Decls/Function.h>
#include <Util/TypeTraits.h>


FunRef::FunRef(const Location& loc, Function* funDecl, Node* resType)
    : Node(classNodeKind(), loc, {resType}, {funDecl})
{
}

Function* FunRef::funDecl() const
{
    ASSERT(referredNodes_.size() == 1);
    return (Function*) referredNodes_[0];
}

void FunRef::dump(ostream& os) const
{
    os << "FunRef(" << funDecl() << ")";
}

void FunRef::doSemanticCheck()
{
    ASSERT(children_.size() == 1);
    Node* resType = children_[0];
    resType->computeType();

    funDecl()->computeType();
    type_ = adjustMode(resType->type(), context_, location_);
}
