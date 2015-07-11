#include <StdInc.h>
#include "FunRef.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Nodes/Decls/Function.h>
#include <Util/TypeTraits.h>


FunRef::FunRef(const Location& loc, Function* funDecl, DynNode* resType)
    : DynNode(classNodeKind(), loc, {resType}, {funDecl})
{
}

Function* FunRef::funDecl() const
{
    ASSERT(data_.referredNodes.size() == 1);
    return (Function*) data_.referredNodes[0];
}

void FunRef::dump(ostream& os) const
{
    os << "FunRef(" << funDecl() << ")";
}

void FunRef::doSemanticCheck()
{
    ASSERT(data_.children.size() == 1);
    DynNode* resType = data_.children[0];
    resType->computeType();

    funDecl()->computeType();
    data_.type = adjustMode(resType->type(), data_.context, data_.location);
}
