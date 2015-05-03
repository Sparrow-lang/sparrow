#include <StdInc.h>
#include "StarExp.h"
#include <Helpers/DeclsHelpers.h>

using namespace SprFrontend;
using namespace Nest;

StarExp::StarExp(const Location& loc, Node* base)
    : Node(loc, {base})
{
}

void StarExp::doSemanticCheck()
{
    Node* base = children_[0];

    // Get the declarations from the base expression
    base->semanticCheck();
    Node* baseExp;
    NodeVector baseDecls = getDeclsFromNode(base, baseExp);
    if ( baseDecls.empty() )
        REP_ERROR(base->location(), "Invalid expression inside star expression (no referred declarations)");

    // Get the referred declarations
    NodeVector decls;
    for ( Node* baseDecl: baseDecls )
    {
        if ( !baseDecl )
            continue;

        // Get the sym tab from the base declaration
        SymTab* baseSymTab = baseDecl->childrenContext()->currentSymTab();
        if ( !baseSymTab )
            continue;

        // Get all the symbols from the symbol table

        // Search in the symbol table of the base for the identifier
        decls = baseSymTab->allEntries();
    }

    if ( decls.empty() )
        REP_ERROR(location_, "No declarations found with the star expression");
    
    // This expands to a NOP
    // Add the referenced declarations as a property to our result
    Node* res = Feather::mkNop(location_);
    res->setProperty(propRefDecls, mkDeclExp(location_, move(decls)));
    setExplanation(res);
}
