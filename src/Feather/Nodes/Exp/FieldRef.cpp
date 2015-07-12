#include <StdInc.h>
#include "FieldRef.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Nodes/Decls/Class.h>
#include <Util/TypeTraits.h>
#include <Util/Decl.h>


FieldRef::FieldRef(const Location& loc, DynNode* obj, DynNode* field)
    : DynNode(classNodeKind(), loc, {obj}, {field})
{
}

DynNode* FieldRef::object() const
{
    return (DynNode*) data_.children[0];
}

DynNode* FieldRef::field() const
{
    return (DynNode*) data_.referredNodes[0];
}

void FieldRef::dump(ostream& os) const
{
    os << "fieldRef(" << data_.children[0] << ", " << data_.referredNodes[0] << ")";
}

void FieldRef::doSemanticCheck()
{
    DynNode* obj = (DynNode*) data_.children[0];
    DynNode* field = (DynNode*) data_.referredNodes[0];
    ASSERT(obj);
    ASSERT(field);
    if ( field->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(data_.location, "FieldRef object needs to point to a Field (node kind: %1%)") % field->nodeKindName();

    // Semantic check the object node - make sure it's a reference to a data type
    obj->semanticCheck();
    ASSERT(obj->type());
    if ( !obj->type() || !obj->type()->hasStorage || obj->type()->numReferences != 1 )
    	REP_ERROR(data_.location, "Field access should be done on a reference to a data type (type: %1%)") % obj->type();
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
    	REP_ERROR(data_.location, "Field '%1%' not found when accessing object") % getName(field);

    // Set the correct type for this node
    ASSERT(field->type());
    ASSERT(field->type()->hasStorage);
    data_.type = getLValueType(field->type());
    data_.type = adjustMode(data_.type, obj->type()->mode, data_.context, data_.location);
}
