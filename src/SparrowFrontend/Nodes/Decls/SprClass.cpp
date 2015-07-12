#include <StdInc.h>
#include "SprClass.h"
#include "GenericClass.h"
#include <NodeCommonsCpp.h>
#include <Helpers/ForEachNodeInNodeList.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/StdDef.h>
#include <IntMods/IntModClassMembers.h>

#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Util/Decl.h>

#include <Nest/Frontend/SourceCode.h>

using namespace SprFrontend;
using namespace Feather;

namespace
{
    /// Get the fields from the symtab of the current class
    /// In the process it might compute the type of the SprVariables
    DynNodeVector getFields(SymTab* curSymTab)
    {
        // Check all the nodes registered in the children context so far to discover the fields
        DynNodeVector fields;
        for ( DynNode* n: curSymTab->allEntriesDyn() )
        {
            switch ( n->nodeKind() )
            {
                case nkFeatherDeclVar:
                    fields.push_back(n);
                    break;
                case nkSparrowDeclSprVariable:
                    fields.push_back(n);
                    break;
            }
        }

        // Sort the fields by location - we need to add them in order
        sort(fields.begin(), fields.end(), [](DynNode* f1, DynNode* f2) { return f1->location() < f2->location(); });

        // Make sure we have only fields
        for ( DynNode*& field: fields )
        {
            field->computeType();
            field = field->explanation();
            if ( field->nodeKind() != nkFeatherDeclVar || !isField(field) )
                field = nullptr;
        }

        // Remove all the nulls
        fields.erase(remove_if(fields.begin(), fields.end(), [](DynNode* n) { return n == nullptr; }), fields.end());

        return fields;
    }
}

SprClass::SprClass(const Location& loc, string name, NodeList* parameters, NodeList* baseClasses, NodeList* children, DynNode* ifClause, AccessType accessType)
    : DynNode(classNodeKind(), loc, {parameters, baseClasses, children, ifClause})
{
    setName(this, move(name));
    setAccessType(this, accessType);
}

NodeList* SprClass::baseClasses() const
{
    ASSERT(data_.children.size() == 4);
    return (NodeList*) data_.children[1];
}
NodeList* SprClass::classChildren() const
{
    ASSERT(data_.children.size() == 4);
    return (NodeList*) data_.children[2];
}

void SprClass::addChild(DynNode* child)
{
    if ( !child )
        return;
    if ( childrenContext() )
        child->setContext(childrenContext());
    if ( data_.type )
        child->computeType();
    if ( !data_.children[2] )
    {
        data_.children[2] = mkNodeList(data_.location, {})->node();
        if ( childrenContext() )
            Nest::setContext(data_.children[2], childrenContext());
    }
    static_cast<NodeList*>((DynNode*) data_.children[2])->addChild(child);
}

void SprClass::dump(ostream& os) const
{
    os << "class " << getName(this);
    if ( data_.children[0] )
        os << "(" << data_.children[0] << ")";
    if ( data_.children[1] )
        os << ": " << data_.children[1];
    os << "\nbody: " << data_.children[2] << "\n";
}

void SprClass::doSetContextForChildren()
{
    addToSymTab(this);
    
    // If we don't have a children context, create one
    if ( !data_.childrenContext )
        data_.childrenContext = data_.context->createChildContext(node(), effectiveEvalMode(this));

    DynNode::doSetContextForChildren();
}

void SprClass::doComputeType()
{
    ASSERT(data_.children.size() == 4);
    NodeList* parameters = (NodeList*) data_.children[0];
    NodeList* baseClasses = (NodeList*) data_.children[1];
    NodeList* children = (NodeList*) data_.children[2];
    DynNode* ifClause = (DynNode*) data_.children[3];

    // Is this a generic?
    if ( parameters && !parameters->children().empty() )
    {
        DynNode* generic = new GenericClass(this, parameters, ifClause);
        setProperty(propResultingDecl, generic);
        setExplanation(generic);
        return;
    }
    if ( ifClause )
        REP_ERROR(data_.location, "If clauses must be applied only to generics; this is not a generic class");

    // Default class members
    if ( !hasProperty(propNoDefault) )
        addModifier(new IntModClassMembers);
    
    DynNode* resultingClass = nullptr;

    // Special case for Type class; re-use the existing StdDef class
    const string* nativeName = getPropertyString(propNativeName);
    if ( nativeName && *nativeName == "Type" )
    {
        resultingClass = StdDef::clsType;
    }

    // Create the resulting Feather.Class object
    if ( !resultingClass )
        resultingClass = Feather::mkClass(data_.location, getName(this), {});
    setShouldAddToSymTab(resultingClass, false);

    // Copy the "native" and "description" properties to the resulting class
    if ( nativeName )
    {
        resultingClass->setProperty(Feather::propNativeName, *nativeName);
    }
    const string* description = getPropertyString(propDescription);
    if ( description )
    {
        resultingClass->setProperty(propDescription, *description);
    }

    setEvalMode(resultingClass, nodeEvalMode(this));
    resultingClass->setChildrenContext(data_.childrenContext);
    resultingClass->setContext(data_.context);
    setProperty(propResultingDecl, resultingClass);

    data_.explanation = resultingClass->node();

    // Check for Std classes
    checkStdClass(resultingClass);
    
    // First check all the base classes
    if ( baseClasses )
    {
        for ( auto& bcName: baseClasses->children() )
        {
            // Make sure the type refers to a class
            TypeRef bcType = getType(bcName);
            if ( !bcType || !bcType->hasStorage )
                REP_ERROR(data_.location, "Invalid base class");
            Class* baseClass = classForType(bcType);
            
            // Compute the type of the base class
            baseClass->computeType();

            // Add the fields of the base class to the resulting basic class
            static_cast<Class*>(resultingClass)->addFields(baseClass->fields());

            // Copy the symbol table entries of the base to this class
            SymTab* ourSymTab = childrenContext()->currentSymTab();
            SymTab* baseSymTab = baseClass->childrenContext()->currentSymTab();
            ourSymTab->copyEntries(baseSymTab);
        }
    }

    // We now have a type - from now on we can safely compute the types of the children
    data_.type = getDataType(static_cast<Class*>(resultingClass));

    // Get the fields from the current class
    DynNodeVector fields = getFields(data_.childrenContext->currentSymTab());
    static_cast<Class*>(resultingClass)->addFields(fields);

    // Check all the children
    if ( children )
    {
        children->computeType();
        checkForAllowedNamespaceChildren(children, true);
    }

    // Take the fields and the methods
    forEachNodeInNodeList(children, [&] (DynNode* child) -> void
    {
        DynNode* p = child->explanation();
        if ( !isField(p) )
        {
            // Methods, generics
            ASSERT(data_.context->sourceCode());
            data_.context->sourceCode()->addAdditionalNode(child);
        }
    });

    // Compute the type for the basic class
    resultingClass->computeType();
}

void SprClass::doSemanticCheck()
{
    computeType();

    Nest::semanticCheck(data_.explanation);

    if ( data_.explanation->nodeKind != nkFeatherDeclClass )
        return; // This should be a generic; there is nothing else to do here

    // Semantic check all the children
    ASSERT(data_.children.size() == 4);
    NodeList* children = (NodeList*) data_.children[2];
    if ( children )
        children->semanticCheck();
}
