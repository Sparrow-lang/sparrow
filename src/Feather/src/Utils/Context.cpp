#include "Feather/src/StdInc.h"
#include "Feather/Utils/Context.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/Decl.h"
#include "Feather/Utils/FeatherNodeKinds.h"
#include "Feather/Utils/Properties.h"
#include "Nest/Api/Node.h"
#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SymTab.h"
#include "Nest/Utils/NodeUtils.h"

using namespace Feather;

Node* Feather::getParentDecl(CompilationContext* context)
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

Node* Feather::getParentFun(CompilationContext* context)
{
    return Nest_ofKind(Nest_explanation(getParentDecl(context)), nkFeatherDeclFunction);
}

Node* Feather::getParentClass(CompilationContext* context)
{
    return Nest_ofKind(Nest_explanation(getParentDecl(context)), nkFeatherDeclClass);
}

Node* Feather::getParentLoop(CompilationContext* context)
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

CompilationContext* Feather::getSymTabContext(CompilationContext* context)
{
    SymTab* parentSymTab = context->currentSymTab;
    Node* n = parentSymTab->node;
    return n ? Nest_childrenContext(n) : nullptr;
}

bool Feather::isLocal(CompilationContext* context)
{
    return nullptr != getParentFun(context);
}

bool Feather::isClassMember(CompilationContext* context)
{
    return nullptr != getParentClass(context);
}
