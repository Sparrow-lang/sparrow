#include <StdInc.h>
#include "Class.h"

#include <Type/DataType.h>
#include <Util/Decl.h>

#include <Nest/Intermediate/CompilationContext.h>

#include <Nest/Common/Diagnostic.h>

using namespace Feather;
using namespace Nest;


Class::Class(const Location& loc, string name, NodeVector fields)
    : Node(loc, move(fields))
{
    // Make sure all the nodes given as parameters have the right kind
    for ( Node* field: fields )
    {
        if ( field->nodeKind() != nkFeatherDeclVar )
            REP_INTERNAL(field->location(), "Node %1% must be a field") % field;
    }

    setName(this, move(name));
}

void Class::addFields(const NodeVector& fields)
{
    // Make sure all the nodes given as parameters have the right kind
    for ( Node* field: fields )
    {
        if ( field->nodeKind() != nkFeatherDeclVar )
            REP_INTERNAL(field->location(), "Node %1% must be a field") % field;
    }
    
    children_.insert(children_.end(), fields.begin(), fields.end());
}

const NodeVector& Class::fields() const
{
    return children_;
}

void Class::dump(ostream& os) const
{
    os << "class(\"" << getName(this) << "\", {" << endl;
    for ( Node* field: children_ )
    {
        os << field << endl;
    }
    os << "})";
}
void Class::doSetContextForChildren()
{
    // If we don't have a children context, create one
    if ( !childrenContext_ )
        childrenContext_ = context_->createChildContext(this, effectiveEvalMode(this));

    // Set the context for all the children
    for ( Node* field: children_ )
        field->setContext(childrenContext_);
    
    addToSymTab(this);
}

void Class::doComputeType()
{
    if ( getName(this).empty() )
        REP_ERROR(location_, "No name given to class");

    // Compute the type for all the fields
    for ( Node* field: children_ )
    {
        try
        {
	        field->computeType();
        }
        catch (const exception&)
        {
            // Don't propagate the errors upward
        }
    }

    // Set the type for this node
    type_ = DataType::get(this, 0, effectiveEvalMode(this));
}

void Class::doSemanticCheck()
{
    // Make sure the type is computed
    computeType();

    // Semantically check all the fields
    for ( Node* field: children_ )
    {
        try
        {
	        field->semanticCheck();
        }
        catch (const exception&)
        {
            // Don't propagate the errors upward
        }
    }
}
