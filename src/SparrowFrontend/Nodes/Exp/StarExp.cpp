#include <StdInc.h>
#include "StarExp.h"
#include <Helpers/DeclsHelpers.h>

using namespace SprFrontend;
using namespace Nest;

StarExp::StarExp(const Location& loc, DynNode* base)
    : DynNode(classNodeKind(), loc, {base})
{
}

void StarExp::doSemanticCheck()
{
    Node* base = data_.children[0];

    // For the base expression allow it to return DeclExp
    Nest::setProperty(base, propAllowDeclExp, 1, true);

    // Get the declarations from the base expression
    Nest::semanticCheck(base);
    Node* baseExp;
    NodeVector baseDecls = getDeclsFromNode(base, baseExp);
    if ( baseDecls.empty() )
        REP_ERROR(base->location, "Invalid expression inside star expression (no referred declarations)");

    // Get the referred declarations
    NodeVector decls;
    for ( Node* baseDecl: baseDecls )
    {
        if ( !baseDecl )
            continue;

        // Get the sym tab from the base declaration
        SymTab* baseSymTab = Nest::childrenContext(baseDecl)->currentSymTab();
        if ( !baseSymTab )
            continue;

        // Get all the symbols from the symbol table

        // Search in the symbol table of the base for the identifier
        decls = baseSymTab->allEntries();
    }

    if ( decls.empty() )
        REP_ERROR(data_.location, "No declarations found with the star expression");
    
    // This expands to a declaration expression
    setExplanation(mkDeclExp(data_.location, move(decls)));
}
