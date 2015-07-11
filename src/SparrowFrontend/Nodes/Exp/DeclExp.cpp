#include <StdInc.h>
#include "DeclExp.h"
#include <NodeCommonsCpp.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Nest;

DeclExp::DeclExp(const Location& loc, DynNodeVector decls, DynNode* baseExp)
    : DynNode(classNodeKind(), loc, {}, move(decls))
{
    data_.referredNodes.insert(data_.referredNodes.begin(), baseExp->node());
}

DynNodeVector DeclExp::decls() const
{
    ASSERT(!data_.referredNodes.empty());
    const DynNodeVector& refNodes = reinterpret_cast<const DynNodeVector&>(data_.referredNodes);
    return DynNodeVector(refNodes.begin()+1, refNodes.end());
}

DynNode* DeclExp::baseExp() const
{
    ASSERT(!data_.referredNodes.empty());
    return (DynNode*) data_.referredNodes[0];
}

void DeclExp::dump(ostream& os) const
{
    os << "DeclRef(";
    for ( size_t i=1; i<data_.referredNodes.size(); ++i )
    {
        if ( i>1 )
            os << ", ";
        os << Feather::getName((DynNode*) data_.referredNodes[i]);
    }
    os << ")";
}

void DeclExp::doSemanticCheck()
{
    // Make sure we computed the type for all the referred nodes
    for ( Node* n: data_.referredNodes )
    {
        if ( n )
            Nest::computeType(n);
    }
    data_.type = Feather::getVoidType(data_.context->evalMode());
}
