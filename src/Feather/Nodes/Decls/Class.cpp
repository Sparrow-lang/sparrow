#include <StdInc.h>
#include "Class.h"

#include <Util/Decl.h>
#include <Feather/FeatherTypes.h>

#include <Nest/Intermediate/CompilationContext.h>

#include <Nest/Common/Diagnostic.h>

using namespace Feather;
using namespace Nest;


Class::Class(const Location& loc, string name, DynNodeVector fields)
    : DynNode(classNodeKind(), loc, move(fields))
{
    // Make sure all the nodes given as parameters have the right kind
    for ( DynNode* field: fields )
    {
        if ( field->nodeKind() != nkFeatherDeclVar )
            REP_INTERNAL(field->location(), "DynNode %1% must be a field") % field;
    }

    setName(this, move(name));
}

void Class::addFields(const DynNodeVector& fields)
{
    // Make sure all the nodes given as parameters have the right kind
    for ( DynNode* field: fields )
    {
        if ( field->nodeKind() != nkFeatherDeclVar )
            REP_INTERNAL(field->location(), "DynNode %1% must be a field") % field;
    }

    const NodeVector& f = reinterpret_cast<const NodeVector&>(fields);
    data_.children.insert(data_.children.end(), f.begin(), f.end());
}

const DynNodeVector& Class::fields() const
{
    return reinterpret_cast<const DynNodeVector&>(data_.children);
}

void Class::dump(ostream& os) const
{
    os << "class(\"" << getName(this) << "\", {" << endl;
    for ( Node* field: data_.children )
    {
        os << field << endl;
    }
    os << "})";
}
void Class::doSetContextForChildren()
{
    // If we don't have a children context, create one
    if ( !data_.childrenContext )
        data_.childrenContext = data_.context->createChildContext(node(), effectiveEvalMode(this));

    // Set the context for all the children
    for ( Node* field: data_.children )
        Nest::setContext(field, data_.childrenContext);
    
    addToSymTab(this);
}

void Class::doComputeType()
{
    if ( getName(this).empty() )
        REP_ERROR(data_.location, "No name given to class");

    // Compute the type for all the fields
    for ( Node* field: data_.children )
    {
        try
        {
            Nest::computeType(field);
        }
        catch (const exception&)
        {
            // Don't propagate the errors upward
        }
    }

    // Set the type for this node
    data_.type = getDataType(node(), 0, effectiveEvalMode(this));
}

void Class::doSemanticCheck()
{
    // Make sure the type is computed
    computeType();

    // Semantically check all the fields
    for ( Node* field: data_.children )
    {
        try
        {
            Nest::semanticCheck(field);
        }
        catch (const exception&)
        {
            // Don't propagate the errors upward
        }
    }
}
