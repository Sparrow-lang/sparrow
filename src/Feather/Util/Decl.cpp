#include <StdInc.h>
#include "Decl.h"
#include <Nodes/FeatherNodeKinds.h>
#include <Nodes/Properties.h>
#include <Nodes/Decls/Class.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Intermediate/SymTab.h>
#include <Nest/Common/Diagnostic.h>

using namespace Feather;

bool Feather::isDecl(Nest::DynNode* node)
{
    switch ( node->explanation()->nodeKind() )
    {
        case nkFeatherDeclFunction:
        case nkFeatherDeclClass:
        case nkFeatherDeclVar:
            return true;
        default:
            return false;
    }
}

bool Feather::isDeclEx(Nest::DynNode* node)
{
    if ( isDecl(node) )
        return true;
    DynNode*const* declPtr = node->getPropertyNode(propResultingDecl);
    return declPtr != nullptr;
}

const string& Feather::getName(const DynNode* decl)
{
    return decl->getCheckPropertyString("name");
}

bool Feather::hasName(const DynNode* decl)
{
    return decl->hasProperty("name");
}

void Feather::setName(DynNode* decl, string name)
{
    decl->setProperty("name", move(name));
}


EvalMode Feather::nodeEvalMode(const DynNode* decl)
{
    const int* val = decl->getPropertyInt("evalMode");
    return val ? (EvalMode) *val : Nest::modeUnspecified;
}
EvalMode Feather::effectiveEvalMode(const DynNode* decl)
{
    EvalMode nodeMode = nodeEvalMode(decl);
    return nodeMode != Nest::modeUnspecified ? nodeMode : decl->context()->evalMode();
}
void Feather::setEvalMode(DynNode* decl, EvalMode val)
{
    decl->setProperty("evalMode", (int) val);

    
    // Sanity check
//    EvalMode curMode = declEvalMode(this);
//    if ( data_.childrenContext && curMode != modeUnspecified && data_.childrenContext->evalMode() != curMode )
//        REP_INTERNAL(data_.location, "Invalid mode set for node; node has %1%, in context %2%") % curMode % data_.childrenContext->evalMode();
}

void Feather::addToSymTab(DynNode* decl)
{
    const int* dontAddToSymTab = decl->getPropertyInt("dontAddToSymTab");
    if ( dontAddToSymTab && *dontAddToSymTab )
        return;
    
    if ( !decl->context() )
        REP_INTERNAL(decl->location(), "Cannot add node %1% to sym-tab: context is not set") % decl->nodeKindName();
    const string& declName = getName(decl);
    if ( declName.empty() )
        REP_INTERNAL(decl->location(), "Cannot add node %1% to sym-tab: no name set") % decl->nodeKindName();
    decl->context()->currentSymTab()->enter(declName, decl);
}

void Feather::setShouldAddToSymTab(DynNode* decl, bool val)
{
    decl->setProperty("dontAddToSymTab", val ? 0 : 1);
}
