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

        /// Getter for the number of parameters of the generic (bound + free)
//        virtual size_t paramsCount() const = 0;
//
//        /// Getter for the idx-th parameter of the generic (bound + free)
//        virtual DynNode* param(size_t idx) const = 0;
//
//        /// Checks if we can instantiate this generic with the given arguments; returns non-null if can instantiate
//        virtual Instantiation* canInstantiate(const DynNodeVector& args) = 0;
//
//        /// Called to actually instantiate a generic; returns the code that needs to be called to access the generic
//        virtual DynNode* instantiateGeneric(const Location& loc, CompilationContext* context, const DynNodeVector& args, Instantiation* instantiation) = 0;

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
