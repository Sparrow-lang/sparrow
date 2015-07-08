#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An compound expression (i.e., a qualified identifier)
    class CompoundExp : public DynNode
    {
        DEFINE_NODE(CompoundExp, nkSparrowExpCompoundExp, "Sparrow.Exp.CompoundExp");

    public:
        CompoundExp(const Location& loc, DynNode* base, string id);

        const string& id() const;

        DynNode* baseDataExp() const;

        string toString() const;
        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
