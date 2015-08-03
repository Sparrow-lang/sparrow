#include <StdInc.h>
#include "Context.h"
#include "Decl.h"
#include <Nodes/Properties.h>
#include <Nodes/Decls/Class.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Intermediate/SymTab.h>

using namespace Feather;

Node* Feather::getParentDecl(CompilationContext* context)
{
    Nest::SymTab* parentSymTab = context->currentSymTab();
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
    Node* decl = Nest::explanation(getParentDecl(context));
    return decl && decl->nodeKind == nkFeatherDeclFunction ? decl : nullptr;
}

Class* Feather::getParentClass(CompilationContext* context)
{
    DynNode* decl = (DynNode*) getParentDecl(context);
    return decl ? decl->explanation()->as<Class>() : nullptr;
}

Node* Feather::getParentLoop(CompilationContext* context)
{
    Nest::SymTab* parentSymTab = context->currentSymTab();
    for ( ; parentSymTab; parentSymTab=parentSymTab->parent() )
    {
        DynNode* n = (DynNode*) parentSymTab->node();
        
        // Do we have a while node?
        if ( n->explanation()->nodeKind() == nkFeatherStmtWhile )
            return n->explanation()->node();

        // Stop if we encounter a declaration
        if ( isDecl(n->node()) )
            return nullptr;
    }
    return nullptr;
}

CompilationContext* Feather::getSymTabContext(CompilationContext* context)
{
    Nest::SymTab* parentSymTab = context->currentSymTab();
    DynNode* n = (DynNode*) parentSymTab->node();
    return n ? n->childrenContext() : nullptr;
}

bool Feather::isLocal(CompilationContext* context)
{
    return nullptr != getParentFun(context);
}

bool Feather::isClassMember(CompilationContext* context)
{
    return nullptr != getParentClass(context);
}
