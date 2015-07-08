#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// DynNode that performs a conversions between two types, without changing the bits of the data
    class Bitcast : public DynNode
    {
        DEFINE_NODE(Bitcast, nkFeatherExpBitcast, "Feather.Exp.Bitcast");

    public:
        Bitcast(const Location& loc, DynNode* destType, DynNode* exp);

        TypeRef destType() const;
        DynNode* exp() const;

    public:
        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
