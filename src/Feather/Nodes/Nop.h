#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that doesn't do anything.
    /// Used for placeholder
    class Nop : public DynNode
    {
        DEFINE_NODE(Nop, nkFeatherNop, "Feather.Nop");

    public:
        Nop(const Location& loc);

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
