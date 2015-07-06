#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// The 'null' literal expression of a given type
    class Null : public DynNode
    {
        DEFINE_NODE(Null, nkFeatherExpNull, "Feather.Exp.Null");

    public:
        Null(const Location& loc, DynNode* typeNode);

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
