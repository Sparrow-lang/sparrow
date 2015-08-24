#include <StdInc.h>
#include "Decl.h"
#include <Nodes/FeatherNodes.h>
#include <Nodes/Properties.h>
#include <Nest/Intermediate/Node.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Intermediate/SymTab.h>
#include <Nest/Common/Diagnostic.hpp>

using namespace Feather;

bool Feather::isDecl(Node* node)
{
    switch ( Nest_explanation(node)->nodeKind - firstFeatherNodeKind )
    {
        case nkRelFeatherDeclFunction:
        case nkRelFeatherDeclClass:
        case nkRelFeatherDeclVar:
            return true;
        default:
            return false;
    }
}

bool Feather::isDeclEx(Node* node)
{
    if ( isDecl(node) )
        return true;
    Node*const* declPtr = Nest_getPropertyNode(node, propResultingDecl);
    return declPtr != nullptr;
}

const string& Feather::getName(const Node* decl)
{
    return Nest_getCheckPropertyString(decl, "name");
}

bool Feather::hasName(const Node* decl)
{
    return Nest_hasProperty(decl, "name");
}

void Feather::setName(Node* decl, string name)
{
    Nest_setProperty(decl, "name", move(name));
}


EvalMode Feather::nodeEvalMode(const Node* decl)
{
    const int* val = Nest_getPropertyInt(decl, "evalMode");
    return val ? (EvalMode) *val : modeUnspecified;
}
EvalMode Feather::effectiveEvalMode(const Node* decl)
{
    EvalMode nodeMode = nodeEvalMode(decl);
    return nodeMode != modeUnspecified ? nodeMode : decl->context->evalMode;
}
void Feather::setEvalMode(Node* decl, EvalMode val)
{
    Nest_setProperty(decl, "evalMode", (int) val);

    
    // Sanity check
//    EvalMode curMode = declEvalMode(this);
//    if ( data_.childrenContext && curMode != modeUnspecified && data_.childrenContext->evalMode() != curMode )
//        REP_INTERNAL(data_.location, "Invalid mode set for node; node has %1%, in context %2%") % curMode % data_.childrenContext->evalMode();
}

void Feather::addToSymTab(Node* decl)
{
    const int* dontAddToSymTab = Nest_getPropertyInt(decl, "dontAddToSymTab");
    if ( dontAddToSymTab && *dontAddToSymTab )
        return;
    
    if ( !decl->context )
        REP_INTERNAL(decl->location, "Cannot add node %1% to sym-tab: context is not set") % Nest_nodeKindName(decl);
    const string& declName = getName(decl);
    if ( declName.empty() )
        REP_INTERNAL(decl->location, "Cannot add node %1% to sym-tab: no name set") % Nest_nodeKindName(decl);
    Nest_symTabEnter(decl->context->currentSymTab, declName.c_str(), decl);
}

void Feather::setShouldAddToSymTab(Node* decl, bool val)
{
    Nest_setProperty(decl, "dontAddToSymTab", val ? 0 : 1);
}
