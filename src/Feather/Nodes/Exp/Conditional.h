#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a conditional expression
    /// The types of the alternatives must be the same; the return type will be the type of the alternatives
    class Conditional : public DynNode
    {
        DEFINE_NODE(Conditional, nkFeatherExpConditional, "Feather.Stmt.Conditional");

    public:
        Conditional(const Location& location, DynNode* condition, DynNode* alt1, DynNode* alt2);

        /// Getter for the condition node
        DynNode* condition() const;

        /// Getter for the first altenative
        DynNode* alternative1() const;

        /// Getter for the second alternative
        DynNode* alternative2() const;

    public:
        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
