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
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Nodes/Decls/Class.h>
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
    DynNodeVector decls = toDyn(data_.context->currentSymTab()->lookup(id));
    if ( decls.empty() )
        REP_ERROR(data_.location, "No declarations found with the given name (%1%)") % id;

    // If at least one decl is a field or method, then transform this into a compound expression starting from 'this'
    bool needsThis = false;
    for ( DynNode* decl: decls )
    {
        decl->computeType();
        DynNode* expl = decl->explanation();
        if ( isField(expl) )
        {
            needsThis = true;
            break;
        }
        SprFunction* fun = decl->as<SprFunction>();
        if ( fun && fun->hasThisParameters() )
        {
            needsThis = true;
            break;
        }
    }
    if ( needsThis )
    {
        // Add 'this' parameter to handle this case
        DynNode* res = mkCompoundExp(data_.location, mkThisExp(data_.location), id);
        setExplanation(res);
        return;
    }

    bool allowDeclExp = 0 != getCheckPropertyInt(propAllowDeclExp);
    DynNode* res = getIdentifierResult(data_.context, data_.location, move(decls), nullptr, allowDeclExp);
    ASSERT(res);
    setExplanation(res);
}

DynNode* SprFrontend::getIdentifierResult(CompilationContext* ctx, const Location& loc, const DynNodeVector& decls, DynNode* baseExp, bool allowDeclExp)
{
    // If this points to one declaration only, try to use that declaration
    if ( decls.size() == 1 )
    {
        DynNode* resDecl = resultingDecl(decls[0]);
        ASSERT(resDecl);
        
        // Check if we can refer to a variable
        if ( resDecl->nodeKind() == nkFeatherDeclVar )
        {
            if ( isField(resDecl) )
            {
                if ( !baseExp )
                    REP_INTERNAL(loc, "No base expression to refer to a field");

                // Make sure the base is a reference
                if ( baseExp->type()->numReferences == 0 )
                {
                    ConversionResult res = canConvert(baseExp, addRef(baseExp->type()));
                    if ( !res )
                        REP_INTERNAL(loc, "Cannot add reference to base of field access");
                    baseExp = res.apply(baseExp);
                    baseExp->computeType();
                }
                DynNode* baseCvt = nullptr;
                doDereference1(baseExp, baseCvt);  // ... but no more than one reference
                baseExp = baseCvt;
                
                return mkFieldRef(loc, baseExp, resDecl);
            }
            return mkVarRef(loc, resDecl);
        }
        
        // If this is a using, get its value
        if ( resDecl->nodeKind() == nkSparrowDeclUsing )
            return resDecl->children()[0];

        // Try to convert this to a type
        TypeRef t = nullptr;
        Class* cls = resDecl->as<Feather::Class>();
        if ( cls )
            t = getDataType(cls->node());
        SprConcept* concept = resDecl->as<SprConcept>();
        if ( concept )
            t = getConceptType(concept);
        if ( t )
            return (DynNode*) createTypeNode(ctx, loc, t);

    }

    // Add the referenced declarations as a property to our result
    if ( allowDeclExp )
        return mkDeclExp(loc, decls, baseExp);

    // If we are here, this identifier could only represent a function application
    DynNode* fapp = mkFunApplication(loc, mkDeclExp(loc, decls, baseExp), nullptr);
    fapp->setContext(ctx);
    fapp->semanticCheck();
    return fapp;
}
