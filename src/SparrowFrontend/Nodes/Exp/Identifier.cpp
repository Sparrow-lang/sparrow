#include <StdInc.h>
#include "Identifier.h"
#include <NodeCommonsCpp.h>
#include <Nodes/Decls/SprFunction.h>
#include <Nodes/Decls/SprConcept.h>
#include <SparrowFrontendTypes.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/Convert.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Nest;
using namespace Feather;

Identifier::Identifier(const Location& loc, string id)
    : DynNode(classNodeKind(), loc)
{
    setProperty("name", move(id));
    setProperty(propAllowDeclExp, 0);
}

const string& Identifier::id() const
{
    return getCheckPropertyString("name");
}

string Identifier::toString() const
{
    return id();
}

void Identifier::doSemanticCheck()
{
    const string& id = getCheckPropertyString("name");

    // Search in the current symbol table for the identifier
    NodeVector decls = data_.context->currentSymTab()->lookup(id);
    if ( decls.empty() )
        REP_ERROR(data_.location, "No declarations found with the given name (%1%)") % id;

    // If at least one decl is a field or method, then transform this into a compound expression starting from 'this'
    bool needsThis = false;
    for ( Node* decl: decls )
    {
        Nest::computeType(decl);
        Node* expl = Nest::explanation(decl);
        if ( isField(expl) )
        {
            needsThis = true;
            break;
        }
        if ( decl && decl->nodeKind == nkSparrowDeclSprFunction && ((SprFunction*) decl)->hasThisParameters() )
        {
            needsThis = true;
            break;
        }
    }
    if ( needsThis )
    {
        // Add 'this' parameter to handle this case
        Node* res = mkCompoundExp(data_.location, mkThisExp(data_.location), id);
        setExplanation(res);
        return;
    }

    bool allowDeclExp = 0 != getCheckPropertyInt(propAllowDeclExp);
    Node* res = getIdentifierResult(data_.context, data_.location, move(decls), nullptr, allowDeclExp);
    ASSERT(res);
    setExplanation(res);
}

Node* SprFrontend::getIdentifierResult(CompilationContext* ctx, const Location& loc, const NodeVector& decls, Node* baseExp, bool allowDeclExp)
{
    // If this points to one declaration only, try to use that declaration
    if ( decls.size() == 1 )
    {
        Node* resDecl = resultingDecl(decls[0]);
        ASSERT(resDecl);
        
        // Check if we can refer to a variable
        if ( resDecl->nodeKind == nkFeatherDeclVar )
        {
            if ( isField(resDecl) )
            {
                if ( !baseExp )
                    REP_INTERNAL(loc, "No base expression to refer to a field");

                // Make sure the base is a reference
                if ( baseExp->type->numReferences == 0 )
                {
                    ConversionResult res = canConvert(baseExp, addRef(baseExp->type));
                    if ( !res )
                        REP_INTERNAL(loc, "Cannot add reference to base of field access");
                    baseExp = res.apply(baseExp);
                    Nest::computeType(baseExp);
                }
                Node* baseCvt = nullptr;
                doDereference1(baseExp, baseCvt);  // ... but no more than one reference
                baseExp = baseCvt;
                
                return mkFieldRef(loc, baseExp, resDecl);
            }
            return mkVarRef(loc, resDecl);
        }
        
        // If this is a using, get its value
        if ( resDecl->nodeKind == nkSparrowDeclUsing )
            return resDecl->children[0];

        // Try to convert this to a type
        TypeRef t = nullptr;
        Node* cls = ofKind(resDecl, nkFeatherDeclClass);
        if ( cls )
            t = getDataType(cls);
        if ( resDecl->nodeKind == nkSparrowDeclSprConcept )
            t = getConceptType((SprConcept*) resDecl);
        if ( t )
            return createTypeNode(ctx, loc, t);

    }

    // Add the referenced declarations as a property to our result
    if ( allowDeclExp )
        return mkDeclExp(loc, decls, baseExp);

    // If we are here, this identifier could only represent a function application
    Node* fapp = mkFunApplication(loc, mkDeclExp(loc, decls, baseExp), nullptr);
    Nest::setContext(fapp, ctx);
    Nest::semanticCheck(fapp);
    return fapp;
}
