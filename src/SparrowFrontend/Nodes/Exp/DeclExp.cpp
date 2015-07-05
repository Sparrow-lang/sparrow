#include <StdInc.h>
#include "DeclExp.h"
#include <NodeCommonsCpp.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Nest;

DeclExp::DeclExp(const Location& loc, NodeVector decls, Node* baseExp)
    : Node(loc, {}, move(decls))
{
    referredNodes_.insert(referredNodes_.begin(), baseExp);
}

NodeVector DeclExp::decls() const
{
    ASSERT(!referredNodes_.empty());
    return NodeVector(referredNodes_.begin()+1, referredNodes_.end());
}

Node* DeclExp::baseExp() const
{
    ASSERT(!referredNodes_.empty());
    return referredNodes_[0];
}

void DeclExp::dump(ostream& os) const
{
    os << "DeclRef(";
    for ( size_t i=1; i<referredNodes_.size(); ++i )
    {
        if ( i>1 )
            os << ", ";
        os << Feather::getName(referredNodes_[i]);
    }
    os << ")";
}

void DeclExp::doSemanticCheck()
{
    // Make sure we computed the type for all the referred nodes
    for ( Node* n: referredNodes_ )
    {
        if ( n )
            n->computeType();
    }
    type_ = Feather::getVoidType(context_->evalMode());
}
