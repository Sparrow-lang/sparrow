#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a standard while statement
    /// The return type will be Void
    class While : public Node
    {
        DEFINE_NODE(While, nkFeatherStmtWhile, "Feather.Stmt.While");

    public:
        While(const Location& location, Node* condition, Node* body, Node* step = nullptr, bool isCt = false);

        /// Getter for the condition node of the while statement
        Node* condition() const;

        /// Getter for the body of the while statement
        Node* body() const;

        /// Getter for the step clause of the while statement
        Node* step() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doSemanticCheck();
    };
}
