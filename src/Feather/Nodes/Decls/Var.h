#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Node that represents a stand-alone (local or global) variable, field or parameter.
    class Var : public Node
    {
        DEFINE_NODE(Var, nkFeatherDeclVar, "Feather.Decls.Var");

    public:
        Var(const Location& loc, string name, Node* typeNode = nullptr, size_t alignment = 0);

        /// Getter for the alignment of the memory. Zero is default (backend-dependent)
        size_t alignment() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
