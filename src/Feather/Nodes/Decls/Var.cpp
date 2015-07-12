#include <StdInc.h>
#include "Var.h"
#include <Util/TypeTraits.h>
#include <Util/Decl.h>

#include <Nest/Intermediate/Type.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Intermediate/SymTab.h>

#include <Nest/Common/Diagnostic.h>

using namespace Feather;


Var::Var(const Location& loc, string name, DynNode* typeNode, size_t alignment)
    : DynNode(classNodeKind(), loc, {typeNode})
{
    setName(this, move(name));
    setProperty("alignment", alignment);
}

size_t Var::alignment() const
{
    return getCheckPropertyInt("alignment");
}

void Var::dump(ostream& os) const
{
    ASSERT(data_.children.size() == 1);
    os << "var(" << getName(this) << ": " << data_.children[0]->type << ")";
}

void Var::doSetContextForChildren()
{
    DynNode::doSetContextForChildren();
    addToSymTab(this);
}

void Var::doComputeType()
{
    // Make sure the variable has a type
    ASSERT(data_.children.size() == 1);
    DynNode* typeNode = (DynNode*) data_.children[0];
    typeNode->computeType();
    data_.type = typeNode->type();
    if ( !data_.type )
        REP_ERROR(data_.location, "No type given for variable");

    // Adjust the mode of the type
    data_.type = adjustMode(data_.type, data_.context, data_.location);
}

void Var::doSemanticCheck()
{
    computeType();

    // Make sure that the type has storage
    if ( !data_.type->hasStorage )
        REP_ERROR(data_.location, "Variable type has no storage (%1%") % data_.type;

    classForTypeRaw(data_.type)->computeType();           // Make sure the type of the class is computed
}
