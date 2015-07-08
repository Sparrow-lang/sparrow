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

    bool isGeneric(const DynNode* node);
    size_t genericParamsCount(const DynNode* node);
    DynNode* genericParam(const DynNode* node, size_t idx);
    Instantiation* genericCanInstantiate(DynNode* node, const DynNodeVector& args);
    DynNode* genericDoInstantiate(DynNode* node, const Location& loc, CompilationContext* context, const DynNodeVector& args, Instantiation* instantiation);
}
