#include <StdInc.h>
#include "Decl.h"
#include <Nodes/FeatherNodeKinds.h>
#include <Nodes/Properties.h>
#include <Nodes/Decls/Class.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Intermediate/SymTab.h>
#include <Nest/Common/Diagnostic.h>

using namespace Feather;

bool Feather::isDecl(Nest::Node* node)
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

bool Feather::isDeclEx(Nest::Node* node)
{
    if ( isDecl(node) )
        return true;
    Node*const* declPtr = node->getPropertyNode(propResultingDecl);
    return declPtr != nullptr;
}

const string& Feather::getName(const Node* decl)
{
    return decl->getCheckPropertyString("name");
}

bool Feather::hasName(const Node* decl)
{
    return decl->hasProperty("name");
}

void Feather::setName(Node* decl, string name)
{
    decl->setProperty("name", move(name));
}


EvalMode Feather::nodeEvalMode(const Node* decl)
{
    const int* val = decl->getPropertyInt("evalMode");
    return val ? (EvalMode) *val : Nest::modeUnspecified;
}
EvalMode Feather::effectiveEvalMode(const Node* decl)
{
    EvalMode nodeMode = nodeEvalMode(decl);
    return nodeMode != Nest::modeUnspecified ? nodeMode : decl->context()->evalMode();
}
void Feather::setEvalMode(Node* decl, EvalMode val)
{
    decl->setProperty("evalMode", (int) val);

    
    // Sanity check
//    EvalMode curMode = declEvalMode(this);
//    if ( childrenContext_ && curMode != modeUnspecified && childrenContext_->evalMode() != curMode )
//        REP_INTERNAL(location_, "Invalid mode set for node; node has %1%, in context %2%") % curMode % childrenContext_->evalMode();
}

void Feather::addToSymTab(Node* decl)
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

void Feather::setShouldAddToSymTab(Node* decl, bool val)
{
    decl->setProperty("dontAddToSymTab", val ? 0 : 1);
}
