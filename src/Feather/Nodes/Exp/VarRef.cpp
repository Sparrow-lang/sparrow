#include <StdInc.h>
#include "VarRef.h"
#include <Feather/FeatherNodeCommonsCpp.h>
#include <Nodes/Decls/Class.h>
#include <Util/TypeTraits.h>
#include <Util/Decl.h>

namespace
{
    bool isField(Node* node)
    {
        if ( node->nodeKind() != nkFeatherDeclVar )
            return false;
        
        // Check the direct parent is a class that contains the given node
        Nest::SymTab* st = node->context()->currentSymTab();
        Node* parent = st ? st->node() : nullptr;
        parent = parent ? parent->explanation() : nullptr;
        Class* cls = parent ? parent->as<Class>() : nullptr;
        if ( cls )
        {
            for ( Node* f: cls->fields() )
                if ( f == node )
                    return true;
        }
        return false;
    }
}


VarRef::VarRef(const Location& loc, Node* var)
    : Node(loc, {}, {var})
{
}

Node* VarRef::variable() const
{
    return referredNodes_[0];
}

void VarRef::dump(ostream& os) const
{
    os << "varRef(" << getName(variable()) << ")";
}

void VarRef::doSemanticCheck()
{
    Node* var = variable();
    ASSERT(var);
    if ( var->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(location_, "VarRef object needs to point to a Field (node kind: %1%)") % var->nodeKindName();
    var->computeType();
    if ( isField(var) )
        REP_INTERNAL(location_, "VarRef used on a field (%1%). Use FieldRef instead") % getName(var);
    if ( !var->type()->hasStorage )
        REP_ERROR(location_, "Variable type is doesn't have a storage type (type: %1%)") % var->type();
    type_ = adjustMode(getLValueType(var->type()), context_, location_);
    checkEvalMode(this, var->type()->mode);
}
