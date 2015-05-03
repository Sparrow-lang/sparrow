#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An compound expression (i.e., a qualified identifier)
    class CompoundExp : public Node
    {
        DEFINE_NODE(CompoundExp, nkSparrowExpCompoundExp, "Sparrow.Exp.CompoundExp");

    public:
        CompoundExp(const Location& loc, Node* base, string id);

        const string& id() const;

        Node* baseDataExp() const;

        virtual string toString() const;
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
