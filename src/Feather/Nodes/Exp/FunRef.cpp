#include <StdInc.h>
#include "FunRef.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Util/TypeTraits.h>


FunRef::FunRef(const Location& loc, Node* funDecl, DynNode* resType)
    : DynNode(classNodeKind(), loc, {resType}, { (DynNode*) funDecl})
{
}

Node* FunRef::funDecl() const
{
    ASSERT(data_.referredNodes.size() == 1);
    return data_.referredNodes[0];
}

void FunRef::dump(ostream& os) const
{
    os << "FunRef(" << funDecl() << ")";
}

void FunRef::doSemanticCheck()
{
    ASSERT(data_.children.size() == 1);
    DynNode* resType = (DynNode*) data_.children[0];
    resType->computeType();

    Nest::computeType(funDecl());
    data_.type = adjustMode(resType->type(), data_.context, data_.location);
}
