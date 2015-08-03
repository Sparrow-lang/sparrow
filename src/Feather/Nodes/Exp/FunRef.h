#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a reference to a function, wrapped in a data type
    /// The type of this node must be a data type, and it's given at construction
    class FunRef : public DynNode
    {
        DEFINE_NODE(FunRef, nkFeatherExpFunRef, "Feather.Exp.FunRef");

    public:
        FunRef(const Location& loc, Node* funDecl, DynNode* resType);

        /// Getter for the function declaration this node refers to
        Node* funDecl() const;

    public:
        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
