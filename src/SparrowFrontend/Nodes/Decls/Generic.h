#pragma once

#include "InstantiationsSet.h"
#include "AccessType.h"
#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Base class for generics declarations
    class Generic : public DynNode
    {
    public:
        Generic(int nodeKind, DynNode* origNode, DynNodeVector genericParams, DynNode* ifClause, AccessType accessType = publicAccess);

    protected:
        void doSemanticCheck();

        const DynNodeVector& genericParams() const;
    };

    bool isGeneric(const Node* node);
    size_t genericParamsCount(const Node* node);
    Node* genericParam(const Node* node, size_t idx);
    Instantiation* genericCanInstantiate(Node* node, const NodeVector& args);
    Node* genericDoInstantiate(Node* node, const Location& loc, CompilationContext* context, const NodeVector& args, Instantiation* instantiation);
}
