#include <StdInc.h>
#include "Context.h"
#include "Decl.h"
#include <Nodes/Properties.h>
#include <Nodes/FeatherNodes.h>
#include <Nest/Intermediate/Node.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Intermediate/SymTab.h>

using namespace Feather;

Node* Feather::getParentDecl(CompilationContext* context)
{
    Nest::SymTab* parentSymTab = context->currentSymTab;
    for ( ; parentSymTab; parentSymTab=parentSymTab->parent() )
    {
        Nest::Node* n = parentSymTab->node();
        if ( n && isDecl(n) )
            return n;
    }
    return nullptr;
}

Node* Feather::getParentFun(CompilationContext* context)
{
    return Nest::ofKind(Nest::explanation(getParentDecl(context)), nkFeatherDeclFunction);
}

Node* Feather::getParentClass(CompilationContext* context)
{
    return Nest::ofKind(Nest::explanation(getParentDecl(context)), nkFeatherDeclClass);
}

Node* Feather::getParentLoop(CompilationContext* context)
{
    Nest::SymTab* parentSymTab = context->currentSymTab;
    for ( ; parentSymTab; parentSymTab=parentSymTab->parent() )
    {
        Node* n = parentSymTab->node();
        
        // Do we have a while node?
        Node* expl = explanation(n);
        if ( expl->nodeKind == nkFeatherStmtWhile )
            return expl;

        // Stop if we encounter a declaration
        if ( isDecl(n) )
            return nullptr;
    }
    return nullptr;
}

CompilationContext* Feather::getSymTabContext(CompilationContext* context)
{
    Nest::SymTab* parentSymTab = context->currentSymTab;
    Node* n = parentSymTab->node();
    return n ? childrenContext(n) : nullptr;
}

bool Feather::isLocal(CompilationContext* context)
{
    return nullptr != getParentFun(context);
}

bool Feather::isClassMember(CompilationContext* context)
{
    return nullptr != getParentClass(context);
}
