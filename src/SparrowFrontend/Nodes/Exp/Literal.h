#pragma once

#include <NodeCommonsH.h>

#include <boost/any.hpp>

namespace SprFrontend
{
    /// An expression representing a Sparrow literal
    class Literal : public DynNode
    {
        DEFINE_NODE(Literal, nkSparrowExpLiteral, "Sparrow.Exp.Literal");

    public:
        Literal(const Location& loc, string litType, string data);

        bool isString() const;
        string asString() const;

        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
