#include <StdInc.h>
#include "DeclsHelpers.h"
#include "SprTypeTraits.h"
#include "ForEachNodeInNodeList.h"
#include <NodeCommonsCpp.h>
#include <Nodes/ModifiersNode.h>
#include <Nodes/Decls/SprConcept.h>
#include <SparrowFrontendTypes.h>
#include <Mods/ModRt.h>
#include <Mods/ModCt.h>
#include <Mods/ModRtCt.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace
{
    void checkNodeAllowed(Node* child, bool insideClass)
    {
        // Check non-declarations
        int nodeKind = explanation(child)->nodeKind;
        if (   nodeKind == nkFeatherNop
            || (!insideClass && nodeKind == nkFeatherBackendCode)
            || (!insideClass && nodeKind == nkFeatherGlobalConstructAction)
            || (!insideClass && nodeKind == nkFeatherGlobalDestructAction)
            )
            return;

        if ( nodeKind == nkSparrowModifiersNode )
        {
            if ( !explanation(child)->nodeError )
                checkNodeAllowed(child->children[0], insideClass);
            return;
        }

        if ( nodeKind == nkFeatherNodeList )
        {
            SprFrontend::checkForAllowedNamespaceChildren(child, insideClass);
            return;
        }

        // Check declarations
        if ( nodeKind == nkFeatherDeclClass
            || nodeKind == nkFeatherDeclFunction
            || nodeKind == nkSparrowDeclGenericClass
            || nodeKind == nkSparrowDeclGenericFunction
            || nodeKind == nkFeatherDeclVar
            )
            return;

        REP_ERROR(child->location, "Invalid node found: %1%") % child;
    }
}

NodeVector SprFrontend::getDeclsFromNode(Node* n, Node*& baseExp)
{
    NodeVector res;
    baseExp = nullptr;
    
    computeType(n);
    n = explanation(n);
    ASSERT(n);

    // Check if the node is a DeclExp, pointing to the actual references
    if ( n->nodeKind == nkSparrowExpDeclExp )
    {
        baseExp = n->referredNodes[0];
        res = NodeVector(n->referredNodes.begin()+1, n->referredNodes.end());
        return res;
    }
    
    // If the node represents a type, try to get the declaration associated with the type
    TypeRef t = tryGetTypeValue(n);
    if ( t )
    {
        // If we have a Type as base, try a constructor/concept call
        if ( t->hasStorage )
        {
            res.push_back(classDecl(t));
        }
        else if ( t->typeKind == typeKindConcept )
        {
            res.push_back(conceptOfType(t)->node());
        }
        else
            t = nullptr;
    }
    
    return res;
}

Node* SprFrontend::resultingDecl(Node* node)
{
    Node*const* res = getPropertyNode(node, propResultingDecl);
    return res ? *res : node;
}

bool SprFrontend::isField(Node* node)
{
    if ( node->nodeKind != nkFeatherDeclVar )
        return false;
    const int* isFieldFlag = getPropertyInt(node, propIsField);
    return isFieldFlag && *isFieldFlag;
}


AccessType SprFrontend::getAccessType(Node* decl)
{
    return (AccessType) getCheckPropertyInt(decl, "spr.accessType");
}

bool SprFrontend::hasAccessType(Node* decl)
{
    return hasProperty(decl, "spr.accessType");
}

void SprFrontend::setAccessType(Node* decl, AccessType accessType)
{
    setProperty(decl, "spr.accessType", (int) accessType);
}

Node* SprFrontend::getResultParam(Node* f)
{
    Node*const* res = getPropertyNode(f, propResultParam);
    return res ? *res : nullptr;
}

void SprFrontend::checkForAllowedNamespaceChildren(Node* children, bool insideClass)
{
    if ( children && children->nodeKind == nkFeatherNodeList )
    {
        for ( Node* child: children->children )
        {
            if ( child )
                checkNodeAllowed(child, insideClass);
        }
    }
}

void SprFrontend::copyModifiersSetMode(Node* src, Node* dest, EvalMode newMode)
{
    dest->modifiers.reserve(src->modifiers.size());
    for ( Modifier* mod: src->modifiers )
    {
        if ( !dynamic_cast<ModRt*>(mod) && !dynamic_cast<ModCt*>(mod) && !dynamic_cast<ModRtCt*>(mod) )
            dest->modifiers.push_back(mod);
    }

    // Make sure we preserve the evaluation mode of the class, after instantiation
    switch ( newMode )
    {
        case Nest::modeRt:
            dest->modifiers.push_back(new ModRt);
            break;
        case Nest::modeCt:
            dest->modifiers.push_back(new ModCt);
            break;
        case Nest::modeRtCt:
            dest->modifiers.push_back(new ModRtCt);
            break;
        default:
            break;
    }
}

bool SprFrontend::funHasThisParameters(Node* fun)
{
    return fun && fun->nodeKind == nkSparrowDeclSprFunction
        && hasProperty(fun, "spr.isMember") && !hasProperty(fun, propIsStatic);
}
