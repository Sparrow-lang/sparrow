#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents continue statement
    /// The return type will be Void
    class Continue : public DynNode
    {
        DEFINE_NODE(Continue, nkFeatherStmtContinue, "Feather.Stmt.Continue");

    public:
        explicit Continue(const Location& location);

        /// Getter for loop node that contains this continue; set during compilation
        DynNode* loop() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
