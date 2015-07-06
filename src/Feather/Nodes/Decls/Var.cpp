#include <StdInc.h>
#include "Var.h"
#include <Util/TypeTraits.h>
#include <Util/Decl.h>

#include <Nest/Intermediate/Type.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Intermediate/SymTab.h>

#include <Nest/Common/Diagnostic.h>

using namespace Feather;


Var::Var(const Location& loc, string name, Node* typeNode, size_t alignment)
    : Node(classNodeKind(), loc, {typeNode})
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
    ASSERT(children_.size() == 1);
    os << "var(" << getName(this) << ": " << children_[0]->type() << ")";
}

void Var::doSetContextForChildren()
{
    Node::doSetContextForChildren();
    addToSymTab(this);
}

void Var::doComputeType()
{
    // Make sure the variable has a type
    ASSERT(children_.size() == 1);
    Node* typeNode = children_[0];
    typeNode->computeType();
    type_ = typeNode->type();
    if ( !type_ )
        REP_ERROR(location_, "No type given for variable");

    // Adjust the mode of the type
    type_ = adjustMode(type_, context_, location_);
}

void Var::doSemanticCheck()
{
    computeType();

    // Make sure that the type has storage
    if ( !type_->hasStorage )
        REP_ERROR(location_, "Variable type has no storage (%1%") % type_;

    classForTypeRaw(type_)->computeType();           // Make sure the type of the class is computed
}
