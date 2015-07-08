#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents break statement
    /// The return type will be Void
    class Break : public DynNode
    {
        DEFINE_NODE(Break, nkFeatherStmtBreak, "Feather.Stmt.Break");

    public:
        explicit Break(const Location& location);

        /// Getter for loop node that contains this break; set during compilation
        DynNode* loop() const;

    public:
        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
