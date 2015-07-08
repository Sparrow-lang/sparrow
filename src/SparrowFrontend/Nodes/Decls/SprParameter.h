#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Class that represents a Sparrow parameter declaration
    class SprParameter : public DynNode
    {
        DEFINE_NODE(SprParameter, nkSparrowDeclSprParameter, "Sparrow.Decl.Parameter");

    public:
        SprParameter(const Location& loc, string name, DynNode* typeNode, DynNode* init = nullptr);
        SprParameter(const Location& loc, string name, TypeRef type, DynNode* init = nullptr);

        DynNode* initValue() const;

        void dump(ostream& os) const;

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();
    };
}
