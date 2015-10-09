#include <StdInc.h>
#include "DeclsHelpers.h"
#include "SprTypeTraits.h"
#include "ForEachNodeInNodeList.h"
#include <NodeCommonsCpp.h>
#include <SparrowFrontendTypes.h>
#include <Nest/Common/PtrArray.h>
#include <Mods.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace
{
    void checkNodeAllowed(Node* child, bool insideClass)
    {
        // Check non-declarations
        int nodeKind = Nest_explanation(child)->nodeKind;
        if (   nodeKind == nkFeatherNop
            || (!insideClass && nodeKind == nkFeatherBackendCode)
            || (!insideClass && nodeKind == nkFeatherGlobalConstructAction)
            || (!insideClass && nodeKind == nkFeatherGlobalDestructAction)
            )
            return;

        if ( nodeKind == nkSparrowModifiersNode )
        {
            if ( !Nest_explanation(child)->nodeError )
                checkNodeAllowed(at(child->children, 0), insideClass);
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
    
    if ( !Nest_computeType(n) )
        return {};        
    n = Nest_explanation(n);
    ASSERT(n);

    // Check if the node is a DeclExp, pointing to the actual references
    if ( n->nodeKind == nkSparrowExpDeclExp )
    {
        baseExp = at(n->referredNodes, 0);
        res = NodeVector(n->referredNodes.beginPtr+1, n->referredNodes.endPtr);
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
            res.push_back(conceptOfType(t));
        }
        else
            t = nullptr;
    }
    
    return res;
}

Node* SprFrontend::resultingDecl(Node* node)
{
    Node*const* res = Nest_getPropertyNode(node, propResultingDecl);
    return res ? *res : node;
}

bool SprFrontend::isField(Node* node)
{
    if ( node->nodeKind != nkFeatherDeclVar )
        return false;
    const int* isFieldFlag = Nest_getPropertyInt(node, propIsField);
    return isFieldFlag && *isFieldFlag;
}


AccessType SprFrontend::getAccessType(Node* decl)
{
    return (AccessType) Nest_getCheckPropertyInt(decl, "spr.accessType");
}

bool SprFrontend::hasAccessType(Node* decl)
{
    return Nest_hasProperty(decl, "spr.accessType");
}

void SprFrontend::setAccessType(Node* decl, AccessType accessType)
{
    Nest_setProperty(decl, "spr.accessType", (int) accessType);
}

Node* SprFrontend::getResultParam(Node* f)
{
    Node*const* res = Nest_getPropertyNode(f, propResultParam);
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
    NestUtils_reservePtrArray((PtrArray*) &dest->modifiers, src->modifiers.endPtr - src->modifiers.beginPtr);
    Modifier** p = src->modifiers.beginPtr;
    for ( ; p!=src->modifiers.endPtr; ++p )
    {
        // TODO (rtct): This is not ok; we should find another way
        if ( !SprFe_isEvalModeMod(*p) )
            Nest_addModifier(dest, *p);
    }

    // Make sure we preserve the evaluation mode of the class, after instantiation
    switch ( newMode )
    {
        case modeRt:
            Nest_addModifier(dest, SprFe_getRtMod());
            break;
        case modeCt:
            Nest_addModifier(dest, SprFe_getCtMod());
            break;
        case modeRtCt:
            Nest_addModifier(dest, SprFe_getRtCtMod());
            break;
        default:
            break;
    }
}

bool SprFrontend::funHasThisParameters(Node* fun)
{
    return fun && fun->nodeKind == nkSparrowDeclSprFunction
        && Nest_hasProperty(fun, "spr.isMember") && !Nest_hasProperty(fun, propIsStatic);
}
