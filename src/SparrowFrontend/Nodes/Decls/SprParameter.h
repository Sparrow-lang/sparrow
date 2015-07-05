#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Class that represents a Sparrow parameter declaration
    class SprParameter : public Node
    {
        DEFINE_NODE(SprParameter, nkSparrowDeclSprParameter, "Sparrow.Decl.Parameter");

    public:
        SprParameter(const Location& loc, string name, Node* typeNode, Node* init = nullptr);
        SprParameter(const Location& loc, string name, TypeRef type, Node* init = nullptr);

        Node* initValue() const;

        virtual void dump(ostream& os) const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
