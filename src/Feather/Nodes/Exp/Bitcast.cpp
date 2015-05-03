#include <StdInc.h>
#include "Bitcast.h"
#include <Feather/FeatherNodeCommonsCpp.h>
#include <Util/TypeTraits.h>


Bitcast::Bitcast(const Location& loc, Node* destType, Node* exp)
    : Node(loc, {exp, destType})
{
    ASSERT(destType);
    ASSERT(exp);
}

Type* Bitcast::destType() const
{
    return children_[1]->type();
}
Node* Bitcast::exp() const
{
    return children_[0];
}

void Bitcast::dump(ostream& os) const
{
    if ( destType() )
        os << "bitcast(" << destType() << ", " << children_[0] << ")";
    else
        os << "bitcast(type(" << children_[1] << "), " << children_[0] << ")";
}

void Bitcast::doSemanticCheck()
{
    children_[0]->semanticCheck();
    children_[1]->computeType();
    Type* tDest = children_[1]->type();

    // Make sure both types have storage
    Type* srcType = children_[0]->type();
    if ( !srcType->hasStorage() )
        REP_ERROR(location_, "The source of a bitcast is not a type with storage (%1%)") % srcType->toString();
    if ( !tDest->hasStorage() )
        REP_ERROR(location_, "The destination type of a bitcast is not a type with storage (%1%)") % tDest->toString();
    
    // Make sure both types are references
    if ( srcType->noReferences() == 0 )
        REP_ERROR(location_, "The source of a bitcast is not a reference (%1%)") % srcType->toString();
    if ( tDest->noReferences() == 0 )
        REP_ERROR(location_, "The destination type of a bitcast is not a reference (%1%)") % tDest->toString();

    type_ = adjustMode(tDest, context_, location_);
}
