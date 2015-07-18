#include <StdInc.h>
#include "VarRef.h"
#include <Feather/FeatherNodeCommonsCpp.h>
#include <Nodes/Decls/Class.h>
#include <Util/TypeTraits.h>
#include <Util/Decl.h>

namespace
{
    bool isField(DynNode* node)
    {
        if ( node->nodeKind() != nkFeatherDeclVar )
            return false;
        
        // Check the direct parent is a class that contains the given node
        Nest::SymTab* st = node->context()->currentSymTab();
        DynNode* parent = st ? (DynNode*) st->node() : nullptr;
        parent = parent ? parent->explanation() : nullptr;
        Class* cls = parent ? parent->as<Class>() : nullptr;
        if ( cls )
        {
            for ( DynNode* f: cls->fields() )
                if ( f == node )
                    return true;
        }
        return false;
    }
}


VarRef::VarRef(const Location& loc, DynNode* var)
    : DynNode(classNodeKind(), loc, {}, {var})
{
}

DynNode* VarRef::variable() const
{
    return (DynNode*) data_.referredNodes[0];
}

void VarRef::dump(ostream& os) const
{
    os << "varRef(" << getName(variable()->node()) << ")";
}

void VarRef::doSemanticCheck()
{
    DynNode* var = variable();
    ASSERT(var);
    if ( var->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(data_.location, "VarRef object needs to point to a Field (node kind: %1%)") % var->nodeKindName();
    var->computeType();
    if ( isField(var) )
        REP_INTERNAL(data_.location, "VarRef used on a field (%1%). Use FieldRef instead") % getName(var->node());
    if ( !var->type()->hasStorage )
        REP_ERROR(data_.location, "Variable type is doesn't have a storage type (type: %1%)") % var->type();
    data_.type = adjustMode(getLValueType(var->type()), data_.context, data_.location);
    checkEvalMode(node(), var->type()->mode);
}
