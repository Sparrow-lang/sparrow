#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An compound expression (i.e., a qualified identifier)
    class CompoundExp : public DynNode
    {
        DEFINE_NODE(CompoundExp, nkSparrowExpCompoundExp, "Sparrow.Exp.CompoundExp");

    public:
        CompoundExp(const Location& loc, Node* base, string id);

        const string& id() const;

        Node* baseDataExp() const;

        string toString() const;
        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
