#include <StdInc.h>
#include "PrepareTranslate.h"
#include "GlobalContext.h"
#include <Module.h>

#include <Nest/Api/Node.h>
#include <Nest/Api/Type.h>
#include <Nest/Api/SourceCode.h>
#include <Nest/Utils/NodeUtils.hpp>
#include <Nest/Utils/Diagnostic.hpp>
#include <Feather/Api/Feather.h>

using namespace LLVMB;
using namespace LLVMB::Tr;

namespace {
//! Checks if the given node is a declaration
bool isDecl(Node* node) {
    switch (node->nodeKind - Feather_getFirstFeatherNodeKind()) {
    case nkRelFeatherDeclClass:
    case nkRelFeatherDeclFunction:
    case nkRelFeatherDeclVar:
        return true;
    default:
        return false;
    }
}
} // namespace

void Tr::prepareTranslate(Node* node, GlobalContext& ctx) {
    if (!node)
        return;

    // Check if we've already prepared this node
    bool nodeIsDecl = isDecl(node);
    if (nodeIsDecl &&
            ctx.targetBackend_.preparedDecls_.find(node) != ctx.targetBackend_.preparedDecls_.end())
        return;

    // Ensure the node is semantically checked
    Node* expl = Nest_semanticCheck(node);
    if (!expl)
        return;

    // Mark this decl as prepared, to avoid cycles
    if (nodeIsDecl)
        ctx.targetBackend_.preparedDecls_.insert(node);

    // Prepare-translate the additional nodes for this node
    for (Node* n : all(node->additionalNodes))
        prepareTranslate(n, ctx);

    // If this node is explained, then just prepare-translate its explanation
    if (node != expl) {
        prepareTranslate(expl, ctx);
        return;
    }

    // Ensure the class corresponding to the type is prepared
    if (node->type && node->type->referredNode)
        prepareTranslate(node->type->referredNode, ctx);

    // Ensure all the children are prepared
    for (Node* n : all(node->children))
        if (n)
            prepareTranslate(n, ctx);

    // Ensure that all the referred nodes that are declarations are prepared
    for (Node* n : all(node->referredNodes)) {
        if (n) {
            Node* expl = Nest_semanticCheck(n);
            if (expl && isDecl(expl))
                prepareTranslate(expl, ctx);
        }
    }
}
