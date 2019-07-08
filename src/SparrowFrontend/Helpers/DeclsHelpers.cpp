#include <StdInc.h>
#include "DeclsHelpers.h"
#include "SprTypeTraits.h"
#include "NodeCommonsCpp.h"
#include "SparrowFrontendTypes.hpp"
#include "Nest/Utils/PtrArray.h"
#include "Mods.h"

using namespace SprFrontend;
using namespace Nest;

namespace {
void checkNodeAllowed(Node* child, bool insideClass) {
    // Check non-declarations
    int nodeKind = Nest_explanation(child)->nodeKind;
    if (nodeKind == nkFeatherNop || (!insideClass && nodeKind == nkFeatherBackendCode) ||
            (!insideClass && nodeKind == nkFeatherGlobalConstructAction) ||
            (!insideClass && nodeKind == nkFeatherGlobalDestructAction))
        return;

    if (nodeKind == nkSparrowModifiersNode) {
        if (!Nest_explanation(child)->nodeError)
            checkNodeAllowed(at(child->children, 0), insideClass);
        return;
    }

    if (nodeKind == nkFeatherNodeList) {
        SprFrontend::checkForAllowedNamespaceChildren(child, insideClass);
        return;
    }

    // Check declarations
    if (nodeKind == nkFeatherDeclClass || (!insideClass && nodeKind == nkFeatherDeclFunction) ||
            nodeKind == nkFeatherDeclVar || (!insideClass && nodeKind == nkSparrowDeclModule) ||
            (!insideClass && nodeKind == nkSparrowDeclPackage) ||
            (!insideClass && nodeKind == nkSparrowDeclSprDatatype) ||
            (!insideClass && nodeKind == nkSparrowDeclSprFunction) ||
            (!insideClass && nodeKind == nkSparrowDeclSprVariable) ||
            (insideClass && nodeKind == nkSparrowDeclSprField) ||
            (!insideClass && nodeKind == nkSparrowDeclSprConcept) ||
            (!insideClass && nodeKind == nkSparrowDeclGenericPackage) ||
            (!insideClass && nodeKind == nkSparrowDeclGenericDatatype) ||
            (!insideClass && nodeKind == nkSparrowDeclGenericFunction) ||
            nodeKind == nkSparrowDeclUsing)
        return;

    REP_ERROR(child->location, "Invalid node found: %1%") % child;
}

/// Removes all the null nodes from the given array.
/// Reduces the size of the array
void removeNullNodes(Nest_NodeArray& nodes) {
    Node** dest = nodes.beginPtr;
    Node** src = nodes.beginPtr;
    while (src != nodes.endPtr) {
        if (!*src)
            ++src;
        else
            *dest++ = *src++;
    }
    nodes.endPtr = dest;
}
} // namespace

NodeVector SprFrontend::getDeclsFromNode(Node* n, Node*& baseExp) {
    NodeVector res;
    baseExp = nullptr;

    if (!Nest_computeType(n))
        return {};
    n = Nest_explanation(n);
    ASSERT(n);

    // Check if the node is a DeclExp, pointing to the actual references
    if (n->nodeKind == nkSparrowExpDeclExp) {
        baseExp = at(n->referredNodes, 0);
        res = NodeVector(n->referredNodes.beginPtr + 1, n->referredNodes.endPtr);
        return res;
    }

    // Check if this is a ModuleRef; if so, get the it's referred content (inner most package)
    if (n->nodeKind == nkSparrowExpModuleRef) {
        if (Nest_hasProperty(n, propResultingDecl))
            res = {Nest_getCheckPropertyNode(n, propResultingDecl)};
        return res;
    }

    // If the node represents a type, try to get the declaration associated with the type
    Type t = tryGetTypeValue(n);
    if (t && t.hasStorage()) {
        res.push_back(t.referredNode());
    }

    return res;
}

Nest_NodeArray SprFrontend::expandDecls(Nest_NodeRange decls, Node* seenFrom) {
    // First, copy the decls as they are in the resulting array
    auto resDecls = Nest_allocNodeArray(size(decls));
    Nest_appendNodesToArray(&resDecls, decls);

    // Iterate over the resulting decls, trying to follow the decls to their
    // final form.
    // Search for patterns 'using(declExp(d))', and replace them with 'd'
    // Mark invalid decls with null
    for (size_t i = 0; i < size(resDecls); i++) {

    run_loop_again:
        Node*& decl = at(resDecls, i);

        // Clear any decl that is not accessible
        if (seenFrom && !canAccessNode(decl, seenFrom)) {
            at(resDecls, i) = nullptr;
            continue;
        }

        // Make sure we can compute the type
        if (!Nest_computeType(decl)) {
            decl = nullptr;
            continue;
        }

        // Already resolved using?
        if (decl->nodeKind == nkSparrowDeclUsing)
            decl = resultingDecl(decl);

        // Make sure we have a valid type for the decl
        if (decl && !Nest_computeType(decl))
            decl = nullptr;

        // Check for 'using(declExp(d))''
        if (decl && decl->nodeKind == nkSparrowDeclUsing) {
            Node* ref = at(decl->children, 0);
            ref = Nest_explanation(ref);
            if (ref->nodeKind == nkSparrowExpDeclExp) {
                // A declExp node always has a place for 'baseExp'
                ASSERT(size(ref->referredNodes) >= 2);

                // Put the first decl in the place of the current one
                decl = at(ref->referredNodes, 1);

                // If this refers to more than 1 decl, put the rest of the
                // decls in our queue
                if (size(ref->referredNodes) > 2) {
                    Nest_NodeRange restDecls = all(ref->referredNodes);
                    restDecls.beginPtr += 2;
                    Nest_appendNodesToArray(&resDecls, restDecls);
                }

                // Make sure we run again for the newly replaced decl
                // NOLINTNEXTLINE
                goto run_loop_again;
            }
        }
    }

    // Make sure the declarations are unique
    sort(resDecls.beginPtr, resDecls.endPtr);
    resDecls.endPtr = unique(resDecls.beginPtr, resDecls.endPtr);

    // Remove the declarations filtered above
    removeNullNodes(resDecls);

    return resDecls;
}

Node* SprFrontend::resultingDecl(Node* node) {
    Node* const* res = Nest_getPropertyNode(node, propResultingDecl);
    return res ? *res : node;
}

bool SprFrontend::canAccessNode(Node* decl, Node* fromNode) {
    ASSERT(decl);
    ASSERT(fromNode);

    // Check if the two nodes have the same compilation unit
    // If yes, then we can access the node
    if (decl->location.sourceCode && decl->location.sourceCode == fromNode->location.sourceCode)
        return true;

    // Otherwise we can only access the node if it's public
    return isPublic(decl);
}

bool SprFrontend::canAccessNode(Node* decl, Nest_SourceCode* fromSourceCode) {
    ASSERT(decl);

    // Check if the two nodes have the same compilation unit
    // If yes, then we can access the node
    if (decl->location.sourceCode && decl->location.sourceCode == fromSourceCode)
        return true;

    // Otherwise we can only access the node if it's public
    return isPublic(decl);
}

bool SprFrontend::isPublic(Node* decl) { return getAccessType(decl) != privateAccess; }

bool SprFrontend::isProtected(Node* decl) { return getAccessType(decl) == protectedAccess; }

AccessType SprFrontend::getAccessType(Node* decl) {
    const int* res = Nest_getPropertyInt(decl, "spr.accessType");
    return res && *res != int(unspecifiedAccess) ? AccessType(*res) : unspecifiedAccess;
}

bool SprFrontend::hasAccessType(Node* decl) {
    const int* res = Nest_getPropertyInt(decl, "spr.accessType");
    return res && *res != int(unspecifiedAccess);
}

void SprFrontend::setAccessType(Node* decl, AccessType accessType) {
    if (accessType != unspecifiedAccess)
        Nest_setPropertyInt(decl, "spr.accessType", accessType);
}

void SprFrontend::deduceAccessType(Node* decl) {
    StringRef name = Nest_getPropertyStringDeref(decl, "name");
    AccessType acc = publicAccess;
    if (name && *name.begin == '_')
        acc = privateAccess;
    else if (name == "ctor" || name == "dtor" || name == "=" || name == "==")
        acc = protectedAccess;
    Nest_setPropertyInt(decl, "spr.accessType", acc);
}

void SprFrontend::copyAccessType(Node* destDecl, Node* srcDecl) {
    const int* res = Nest_getPropertyInt(srcDecl, "spr.accessType");
    if (res)
        Nest_setPropertyInt(destDecl, "spr.accessType", *res);
}

Node* SprFrontend::getResultParam(Node* f) {
    Node* const* res = Nest_getPropertyNode(f, propResultParam);
    return res ? *res : nullptr;
}

void SprFrontend::checkForAllowedNamespaceChildren(Node* children, bool insideClass) {
    if (children && children->nodeKind == nkFeatherNodeList) {
        for (Node* child : children->children) {
            if (child)
                checkNodeAllowed(child, insideClass);
        }
    }
}

void SprFrontend::copyModifiersSetMode(Node* src, Node* dest, EvalMode newMode) {
    NestUtils_reservePtrArray(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            (NestUtils_PtrArray*)&dest->modifiers, src->modifiers.endPtr - src->modifiers.beginPtr);
    Nest_Modifier** p = src->modifiers.beginPtr;
    for (; p != src->modifiers.endPtr; ++p) {
        // TODO (rtct): This is not ok; we should find another way
        if (!SprFe_isEvalModeMod(*p))
            Nest_addModifier(dest, *p);
    }

    // Make sure we preserve the evaluation mode of the class, after instantiation
    switch (newMode) {
    case modeRt:
        Nest_addModifier(dest, SprFe_getRtMod());
        break;
    case modeCt:
        Nest_addModifier(dest, SprFe_getCtMod());
        break;
    default:
        break;
    }
}

void SprFrontend::copyOverloadPrio(Node* src, Node* dest) {
    auto overloadPrio = Nest_getPropertyInt(src, propOverloadPrio);
    if (overloadPrio)
        Nest_setPropertyExplInt(dest, propOverloadPrio, *overloadPrio);
}

bool SprFrontend::funHasThisParameters(Node* fun) {
    return fun && fun->nodeKind == nkSparrowDeclSprFunction &&
           Nest_hasProperty(fun, propThisParamIdx);
}

int SprFrontend::getThisParamIdx(Node* fun) {
    if (!fun || fun->nodeKind != nkSparrowDeclSprFunction)
        return -1;
    const int* val = Nest_getPropertyInt(fun, propThisParamIdx);
    return val ? *val : -1;
}

CompilationContext* SprFrontend::classContext(Node* cls) {
    CompilationContext* res = cls->context;
    while (res && res->parent) {
        Node* n = res->currentSymTab->node;
        if (n && n->nodeKind != nkSparrowDeclSprDatatype &&
                n->nodeKind != nkSparrowDeclGenericDatatype &&
                n->nodeKind != Feather_getFirstFeatherNodeKind() + nkRelFeatherDeclClass)
            break;

        res = res->parent;
    }
    return res ? res : cls->context;
}

Nest_NodeArray SprFrontend::getClassAssociatedDecls(Node* cls, const char* name) {
    return Nest_symTabLookupCurrent(classContext(cls)->currentSymTab, name);
}
