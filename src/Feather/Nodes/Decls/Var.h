#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// DynNode that represents a stand-alone (local or global) variable, field or parameter.
    class Var : public DynNode
    {
        DEFINE_NODE(Var, nkFeatherDeclVar, "Feather.Decls.Var");

    public:
        Var(const Location& loc, string name, DynNode* typeNode = nullptr, size_t alignment = 0);

        /// Getter for the alignment of the memory. Zero is default (backend-dependent)
        size_t alignment() const;

    public:
        void dump(ostream& os) const;

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();
    };
}
