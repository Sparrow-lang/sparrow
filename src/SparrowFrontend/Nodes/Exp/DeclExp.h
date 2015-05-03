#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An expression that just refers to declarations
    /// It returns a Void type, and it doesn't expand to anything
    /// It's used just as a mean to pass a set of declarations around. For example, the Identifier and CompoundExp nodes
    /// might generate this, and FunApplication may consume this node
    class DeclExp : public Node
    {
        DEFINE_NODE(DeclExp, nkSparrowExpDeclExp, "Sparrow.Exp.DeclExp");

    public:
        DeclExp(const Location& loc, NodeVector decls, Node* baseExp = nullptr);

        NodeVector decls() const;
        Node* baseExp() const;

    public:
        virtual void dump(ostream& os) const;
    protected:
        virtual void doSemanticCheck();
    };
}
