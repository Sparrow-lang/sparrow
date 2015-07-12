#include <StdInc.h>
#include "Bitcast.h"
#include <Feather/FeatherNodeCommonsCpp.h>
#include <Util/TypeTraits.h>


Bitcast::Bitcast(const Location& loc, DynNode* destType, DynNode* exp)
    : DynNode(classNodeKind(), loc, {exp, destType})
{
    ASSERT(destType);
    ASSERT(exp);
}

TypeRef Bitcast::destType() const
{
    return data_.children[1]->type;
}
DynNode* Bitcast::exp() const
{
    return (DynNode*) data_.children[0];
}

void Bitcast::dump(ostream& os) const
{
    if ( destType() )
        os << "bitcast(" << destType() << ", " << data_.children[0] << ")";
    else
        os << "bitcast(type(" << data_.children[1] << "), " << data_.children[0] << ")";
}

void Bitcast::doSemanticCheck()
{
    Nest::semanticCheck(data_.children[0]);
    Nest::computeType(data_.children[1]);
    TypeRef tDest = data_.children[1]->type;

    // Make sure both types have storage
    TypeRef srcType = data_.children[0]->type;
    if ( !srcType->hasStorage )
        REP_ERROR(data_.location, "The source of a bitcast is not a type with storage (%1%)") % srcType;
    if ( !tDest->hasStorage )
        REP_ERROR(data_.location, "The destination type of a bitcast is not a type with storage (%1%)") % tDest;
    
    // Make sure both types are references
    if ( srcType->numReferences == 0 )
        REP_ERROR(data_.location, "The source of a bitcast is not a reference (%1%)") % srcType;
    if ( tDest->numReferences == 0 )
        REP_ERROR(data_.location, "The destination type of a bitcast is not a reference (%1%)") % tDest;

    data_.type = adjustMode(tDest, data_.context, data_.location);
}
