#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Node that contains backend code written in a backend defined manner.
    /// This node always compile successfully, and always has the type of Void. However, this node may represent various
    /// backend constructs, including function definitions, expressions, etc.
    class BackendCode : public Node
    {
        DEFINE_NODE(BackendCode, nkFeatherBackendCode, "Feather.BackendCode");

    public:
        BackendCode(const Location& location, string code, EvalMode evalMode = Nest::modeRt);

        /// Getter the for the actual backend code
        string code() const;

        /// Getter for the evaluation mode of this backend code
        EvalMode evalMode() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
