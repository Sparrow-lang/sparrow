#include <StdInc.h>
#include "FieldRef.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Nodes/Decls/Class.h>
#include <Type/LValueType.h>
#include <Util/TypeTraits.h>
#include <Util/Decl.h>


FieldRef::FieldRef(const Location& loc, Node* obj, Node* field)
    : Node(loc, {obj}, {field})
{
}

Node* FieldRef::object() const
{
    return children_[0];
}

Node* FieldRef::field() const
{
    return referredNodes_[0];
}

void FieldRef::dump(ostream& os) const
{
    os << "fieldRef(" << children_[0] << ", " << referredNodes_[0] << ")";
}

void FieldRef::doSemanticCheck()
{
    Node* obj = children_[0];
    Node* field = referredNodes_[0];
    ASSERT(obj);
    ASSERT(field);
    if ( field->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(location_, "FieldRef object needs to point to a Field (node kind: %1%)") % field->nodeKindName();

    // Semantic check the object node - make sure it's a reference to a data type
    obj->semanticCheck();
    ASSERT(obj->type());
    if ( !obj->type() || !obj->type()->hasStorage() || obj->type()->noReferences() != 1 )
    	REP_ERROR(location_, "Field access should be done on a reference to a data type (type: %1%)") % obj->type()->toString();
    Class* cls = classForType(obj->type());
    ASSERT(cls);
    cls->computeType();

    // Compute the type of the field
    field->computeType();

    // Make sure that the type of a object is a data type that refers to a class the contains the given field
    bool fieldFound = false;
    for ( auto field: cls->fields() )
    {
        if ( &*field == field )
        {
            fieldFound = true;
            break;
        }
    }
    if ( !fieldFound )
    	REP_ERROR(location_, "Field '%1%' not found when accessing object") % getName(field);

    // Set the correct type for this node
    ASSERT(field->type());
    ASSERT(field->type()->hasStorage());
    type_ = LValueType::get(field->type());
    type_ = adjustMode(type_, obj->type()->mode(), context_, location_);
}
