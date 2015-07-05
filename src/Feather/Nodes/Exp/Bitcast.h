#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Node that performs a conversions between two types, without changing the bits of the data
    class Bitcast : public Node
    {
        DEFINE_NODE(Bitcast, nkFeatherExpBitcast, "Feather.Exp.Bitcast");

    public:
        Bitcast(const Location& loc, Node* destType, Node* exp);

        TypeRef destType() const;
        Node* exp() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
