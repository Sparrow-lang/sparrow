#include <StdInc.h>
#include "DeclsHelpers.h"
#include "SprTypeTraits.h"
#include "ForEachNodeInNodeList.h"
#include <NodeCommonsCpp.h>
#include <Nodes/ModifiersNode.h>
#include <Nodes/Exp/DeclExp.h>
#include <SparrowFrontendTypes.h>
#include <Mods/ModRt.h>
#include <Mods/ModCt.h>
#include <Mods/ModRtCt.h>

#include <Feather/Nodes/Decls/Class.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace
{
    void checkNodeAllowed(DynNode* child, bool insideClass)
    {
        // Check non-declarations
        int nodeKind = child->explanation()->nodeKind();
        if (   nodeKind == nkFeatherNop
            || (!insideClass && nodeKind == nkFeatherBackendCode)
            || (!insideClass && nodeKind == nkFeatherGlobalConstructAction)
            || (!insideClass && nodeKind == nkFeatherGlobalDestructAction)
            )
            return;

        if ( nodeKind == nkSparrowModifiersNode )
        {
            if ( !child->explanation()->hasError() )
                checkNodeAllowed(static_cast<ModifiersNode*>(child)->base(), insideClass);
            return;
        }

        if ( nodeKind == nkFeatherNodeList )
        {
            SprFrontend::checkForAllowedNamespaceChildren(child->node(), insideClass);
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

        REP_ERROR(child->location(), "Invalid node found (%1%)") % child->toString();
    }
}

DynNodeVector SprFrontend::getDeclsFromNode(DynNode* n, DynNode*& baseExp)
{
    DynNodeVector res;
    baseExp = nullptr;
    
    n->computeType();
    n = n->explanation();
    ASSERT(n);

    // Check if the node is a DeclExp, pointing to the actual references
    DeclExp* declExp = n->as<DeclExp>();
    if ( declExp )
    {
        baseExp = declExp->baseExp();
        res = declExp->decls();
        return res;
    }
    
    // If the node represents a type, try to get the declaration associated with the type
    TypeRef t = tryGetTypeValue(n);
    if ( t )
    {
        // If we have a Type as base, try a constructor/concept call
        if ( t->hasStorage )
        {
            res.push_back((DynNode*) classDecl(t));
        }
        else if ( t->typeKind == typeKindConcept )
        {
            res.push_back((DynNode*) conceptOfType(t));
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

DynNode* SprFrontend::resultingDecl(DynNode* node)
{
    DynNode*const* res = node->getPropertyDynNode(propResultingDecl);
    return res ? *res : node;
}

bool SprFrontend::isField(DynNode* node)
{
    if ( node->nodeKind() != nkFeatherDeclVar )
        return false;
    const int* isFieldFlag = node->getPropertyInt(propIsField);
    return isFieldFlag && *isFieldFlag;
}


AccessType SprFrontend::getAccessType(DynNode* decl)
{
    return (AccessType) decl->getCheckPropertyInt("spr.accessType");
}

bool SprFrontend::hasAccessType(DynNode* decl)
{
    return decl->hasProperty("spr.accessType");
}

void SprFrontend::setAccessType(DynNode* decl, AccessType accessType)
{
    decl->setProperty("spr.accessType", (int) accessType);
}

Node* SprFrontend::getResultParam(Node* f)
{
    Node*const* res = getPropertyNode(f, propResultParam);
    return res ? *res : nullptr;
}
DynNode* SprFrontend::getResultParam(DynNode* f)
{
    DynNode*const* res = f->getPropertyDynNode(propResultParam);
    return res ? *res : nullptr;
}

void SprFrontend::checkForAllowedNamespaceChildren(Node* children, bool insideClass)
{
    if ( children && children->nodeKind == nkFeatherNodeList )
    {
        for ( Node* child: children->children )
        {
            if ( child )
                checkNodeAllowed((DynNode*) child, insideClass);
        }
    }
}

void SprFrontend::copyModifiersSetMode(DynNode* src, DynNode* dest, EvalMode newMode)
{
    dest->modifiers().reserve(src->modifiers().size());
    for ( Modifier* mod: src->modifiers() )
    {
        if ( !dynamic_cast<ModRt*>(mod) && !dynamic_cast<ModCt*>(mod) && !dynamic_cast<ModRtCt*>(mod) )
            dest->modifiers().push_back(mod);
    }

    // Make sure we preserve the evaluation mode of the class, after instantiation
    switch ( newMode )
    {
        case Nest::modeRt:
            dest->modifiers().push_back(new ModRt);
            break;
        case Nest::modeCt:
            dest->modifiers().push_back(new ModCt);
            break;
        case Nest::modeRtCt:
            dest->modifiers().push_back(new ModRtCt);
            break;
        default:
            break;
    }
}
