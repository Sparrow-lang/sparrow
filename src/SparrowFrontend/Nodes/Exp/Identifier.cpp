#include <StdInc.h>
#include "Identifier.h"
#include <NodeCommonsCpp.h>
#include <Nodes/Decls/SprFunction.h>
#include <Nodes/Decls/SprConcept.h>
#include <Type/ConceptType.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/Convert.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Type/DataType.h>

using namespace SprFrontend;
using namespace Nest;
using namespace Feather;

namespace
{
    Node* valueIfValidImpl(const Location& loc, Node* argMain, Node* argDefault)
    {
        // Try to compile the main argument, see if compiles
        bool isValid = false;
        Nest::Common::DiagnosticSeverity level = Nest::theCompiler().diagnosticReporter().severityLevel();
        try
        {
            Nest::theCompiler().diagnosticReporter().setSeverityLevel(Nest::Common::diagInternalError);
            argMain->semanticCheck();
            isValid = !argMain->hasError();
        }
        catch (...)
        {
        }
        Nest::theCompiler().diagnosticReporter().setSeverityLevel(level);

        return isValid ? argMain : argDefault;
    }
}

Identifier::Identifier(const Location& loc, string id)
    : Node(loc)
{
    setProperty("name", move(id));
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
    NodeVector decls = context_->currentSymTab()->lookup(id);
    if ( decls.empty() )
        REP_ERROR(location_, "No declarations found with the given name (%1%)") % id;

    // If at least one decl is a field or method, then transform this into a compound expression starting from 'this'
    bool needsThis = false;
    for ( Node* decl: decls )
    {
        decl->computeType();
        Node* expl = decl->explanation();
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
        Node* res = mkCompoundExp(location_, mkThisExp(location_), id);
        setExplanation(res);
        return;
    }
    
    Node* res = getIdentifierResult(context_, location_, move(decls), nullptr);
    ASSERT(res);
    setExplanation(res);
}

Node* SprFrontend::getIdentifierResult(CompilationContext* ctx, const Location& loc, const NodeVector& decls, Node* baseExp)
{
    // If this points to one declaration only, try to use that declaration
    if ( decls.size() == 1 )
    {
        Node* resDecl = resultingDecl(decls[0]);
        ASSERT(resDecl);
        
        // Check if we can refer to a variable
        if ( resDecl->nodeKind() == nkFeatherDeclVar )
        {
            if ( isField(resDecl) )
            {
                if ( !baseExp )
                    REP_INTERNAL(loc, "No base expression to refer to a field");

                // Make sure the base is a reference
                if ( baseExp->type()->noReferences() == 0 )
                {
                    ConversionResult res = canConvert(baseExp, addRef(baseExp->type()));
                    if ( !res )
                        REP_INTERNAL(loc, "Cannot add reference to base of field access");
                    baseExp = res.apply(baseExp);
                    baseExp->computeType();
                }
                Node* baseCvt = nullptr;
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
        Type* t = nullptr;
        Class* cls = resDecl->as<Feather::Class>();
        if ( cls )
            t = DataType::get(cls);
        SprConcept* concept = resDecl->as<SprConcept>();
        if ( concept )
            t = ConceptType::get(concept);
        if ( t )
            return (Node*) createTypeNode(ctx, loc, t);

    }

    // Add the referenced declarations as a property to our result
    Node* declExp = mkDeclExp(loc, decls, baseExp);
    Node* res = declExp;
    declExp->setProperty(propRefDecls, declExp);    // TODO: avoid this reference to self

    // If this refers to a function without an arguments, try to create a function call
    if ( decls.size() == 1 )
    {
        Node* resDecl = resultingDecl(decls[0]);
        Function* f = resDecl->explanation()->as<Function>();
        if ( f )
        {
            // Check if the function has no parameters (or we can call it with a base expression)
            int numParams = f->numParameters();
            if ( f->hasProperty(propResultParam) )
                --numParams;
            if ( numParams == 0 || (numParams == 1 && baseExp != nullptr) )
            {
                // Now try to call it, see if we get any errors. If so, fall back
                Node* fapp = mkFunApplication(loc, res, nullptr);
                fapp->setContext(ctx);
                fapp = valueIfValidImpl(loc, fapp, res);
                if ( fapp )
                {
                    fapp->semanticCheck();
                    res = fapp;
                    res->explanation()->setProperty(propRefDecls, declExp);
                }
            }
        }
    }

    return res;
}
