#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a conditional expression
    /// The types of the alternatives must be the same; the return type will be the type of the alternatives
    class Conditional : public Node
    {
        DEFINE_NODE(Conditional, nkFeatherExpConditional, "Feather.Stmt.Conditional");

    public:
        Conditional(const Location& location, Node* condition, Node* alt1, Node* alt2);

        /// Getter for the condition node
        Node* condition() const;

        /// Getter for the first altenative
        Node* alternative1() const;

        /// Getter for the second alternative
        Node* alternative2() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
