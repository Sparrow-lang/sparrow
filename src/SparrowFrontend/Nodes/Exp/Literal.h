#pragma once

#include <NodeCommonsH.h>

#include <boost/any.hpp>

namespace SprFrontend
{
    /// An expression representing a Sparrow literal
    class Literal : public Node
    {
        DEFINE_NODE(Literal, nkSparrowExpLiteral, "Sparrow.Exp.Literal");

    public:
        Literal(const Location& loc, string litType, string data);

        bool isString() const;
        string asString() const;

        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
