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

        virtual string toString() const;
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
