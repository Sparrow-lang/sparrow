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

TypeRef Bitcast::destType() const
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
    TypeRef tDest = children_[1]->type();

    // Make sure both types have storage
    TypeRef srcType = children_[0]->type();
    if ( !srcType->hasStorage )
        REP_ERROR(location_, "The source of a bitcast is not a type with storage (%1%)") % srcType;
    if ( !tDest->hasStorage )
        REP_ERROR(location_, "The destination type of a bitcast is not a type with storage (%1%)") % tDest;
    
    // Make sure both types are references
    if ( srcType->numReferences == 0 )
        REP_ERROR(location_, "The source of a bitcast is not a reference (%1%)") % srcType;
    if ( tDest->numReferences == 0 )
        REP_ERROR(location_, "The destination type of a bitcast is not a reference (%1%)") % tDest;

    type_ = adjustMode(tDest, context_, location_);
}
