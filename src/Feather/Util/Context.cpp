#include <StdInc.h>
#include "Context.h"
#include "Decl.h"
#include <Nodes/Properties.h>
#include <Nodes/Decls/Function.h>
#include <Nodes/Decls/Class.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Intermediate/SymTab.h>

using namespace Feather;

DynNode* Feather::getParentDecl(CompilationContext* context)
{
    Nest::SymTab* parentSymTab = context->currentSymTab();
    for ( ; parentSymTab; parentSymTab=parentSymTab->parent() )
    {
        DynNode* n = parentSymTab->node();
        if ( n && isDecl(n) )
            return n;
    }
    return nullptr;
}

Function* Feather::getParentFun(CompilationContext* context)
{
    DynNode* decl = getParentDecl(context);
    return decl ? decl->explanation()->as<Function>() : nullptr;
}

Class* Feather::getParentClass(CompilationContext* context)
{
    DynNode* decl = getParentDecl(context);
    return decl ? decl->explanation()->as<Class>() : nullptr;
}

DynNode* Feather::getParentLoop(CompilationContext* context)
{
    Nest::SymTab* parentSymTab = context->currentSymTab();
    for ( ; parentSymTab; parentSymTab=parentSymTab->parent() )
    {
        DynNode* n = parentSymTab->node();
        
        // Do we have a while node?
        if ( n->explanation()->nodeKind() == nkFeatherStmtWhile )
            return n->explanation();

        // Stop if we encounter a declaration
        if ( isDecl(n) )
            return nullptr;
    }
    return nullptr;
}

CompilationContext* Feather::getSymTabContext(CompilationContext* context)
{
    Nest::SymTab* parentSymTab = context->currentSymTab();
    DynNode* n = parentSymTab->node();
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
