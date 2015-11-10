#include "Feather/src/StdInc.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Feather/Api/Feather.h"
#include "Nest/Api/Node.h"
#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SymTab.h"
#include "Nest/Utils/NodeUtils.h"

using namespace Feather;

Node* _getParentDecl(CompilationContext* context)
{
    SymTab* parentSymTab = context->currentSymTab;
    for ( ; parentSymTab; parentSymTab=parentSymTab->parent )
    {
        Node* n = parentSymTab->node;
        if ( n && isDecl(n) )
            return n;
    }
    return nullptr;
}

Node* Feather_getParentFun(CompilationContext* context)
{
    return Nest_ofKind(Nest_explanation(_getParentDecl(context)), nkFeatherDeclFunction);
}

Node* Feather_getParentClass(CompilationContext* context)
{
    return Nest_ofKind(Nest_explanation(_getParentDecl(context)), nkFeatherDeclClass);
}

Node* Feather_getParentLoop(CompilationContext* context)
{
    SymTab* parentSymTab = context->currentSymTab;
    for ( ; parentSymTab; parentSymTab=parentSymTab->parent )
    {
        Node* n = parentSymTab->node;
        
        // Do we have a while node?
        Node* expl = Nest_explanation(n);
        if ( expl->nodeKind == nkFeatherStmtWhile )
            return expl;

        // Stop if we encounter a declaration
        if ( isDecl(n) )
            return nullptr;
    }
    return nullptr;
}
