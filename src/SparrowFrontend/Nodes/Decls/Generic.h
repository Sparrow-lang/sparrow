#pragma once

#include "InstantiationsSet.h"
#include "AccessType.h"
#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Base class for generics declarations
    class Generic : public Node
    {
    public:
        Generic(int nodeKind, Node* origNode, NodeVector genericParams, Node* ifClause, AccessType accessType = publicAccess);
        virtual ~Generic() = 0;

        /// Getter for the number of parameters of the generic (bound + free)
        virtual size_t paramsCount() const = 0;

        /// Getter for the idx-th parameter of the generic (bound + free)
        virtual Node* param(size_t idx) const = 0;

        /// Checks if we can instantiate this generic with the given arguments; retuns non-null if can instantiate
        virtual Instantiation* canInstantiate(const NodeVector& args) = 0;

        /// Called to actually instantiate a generic; returns the code that needs to be called to access the generic
        virtual Node* instantiateGeneric(const Location& loc, CompilationContext* context, const NodeVector& args, Instantiation* instantiation) = 0;

    protected:
        virtual void doSemanticCheck();

        const NodeVector& genericParams() const;
    };
}
