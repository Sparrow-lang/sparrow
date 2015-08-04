#include <StdInc.h>
#include "Decl.h"
#include <Nodes/FeatherNodes.h>
#include <Nodes/Properties.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Intermediate/SymTab.h>
#include <Nest/Common/Diagnostic.h>

using namespace Feather;
using Nest::Node;

bool Feather::isDecl(Node* node)
{
    switch ( explanation(node)->nodeKind - firstFeatherNodeKind )
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
    Nest::Node*const* declPtr = Nest::getPropertyNode(node, propResultingDecl);
    return declPtr != nullptr;
}

const string& Feather::getName(const Node* decl)
{
    return Nest::getCheckPropertyString(decl, "name");
}

bool Feather::hasName(const Node* decl)
{
    return Nest::hasProperty(decl, "name");
}

void Feather::setName(Node* decl, string name)
{
    Nest::setProperty(decl, "name", move(name));
}


EvalMode Feather::nodeEvalMode(const Node* decl)
{
    const int* val = Nest::getPropertyInt(decl, "evalMode");
    return val ? (EvalMode) *val : Nest::modeUnspecified;
}
EvalMode Feather::effectiveEvalMode(const Node* decl)
{
    EvalMode nodeMode = nodeEvalMode(decl);
    return nodeMode != Nest::modeUnspecified ? nodeMode : decl->context->evalMode();
}
void Feather::setEvalMode(Node* decl, EvalMode val)
{
    Nest::setProperty(decl, "evalMode", (int) val);

    
    // Sanity check
//    EvalMode curMode = declEvalMode(this);
//    if ( data_.childrenContext && curMode != modeUnspecified && data_.childrenContext->evalMode() != curMode )
//        REP_INTERNAL(data_.location, "Invalid mode set for node; node has %1%, in context %2%") % curMode % data_.childrenContext->evalMode();
}

void Feather::addToSymTab(Node* decl)
{
    const int* dontAddToSymTab = Nest::getPropertyInt(decl, "dontAddToSymTab");
    if ( dontAddToSymTab && *dontAddToSymTab )
        return;
    
    if ( !decl->context )
        REP_INTERNAL(decl->location, "Cannot add node %1% to sym-tab: context is not set") % Nest::nodeKindName(decl);
    const string& declName = getName(decl);
    if ( declName.empty() )
        REP_INTERNAL(decl->location, "Cannot add node %1% to sym-tab: no name set") % Nest::nodeKindName(decl);
    decl->context->currentSymTab()->enter(declName, decl);
}

void Feather::setShouldAddToSymTab(Node* decl, bool val)
{
    Nest::setProperty(decl, "dontAddToSymTab", val ? 0 : 1);
}
