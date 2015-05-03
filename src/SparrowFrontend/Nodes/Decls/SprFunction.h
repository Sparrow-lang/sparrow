#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>

FWD_CLASS1(Feather, Function);

namespace SprFrontend
{
    /// Class that represents a Sparrow namespace declaration (or compilation unit)
    class SprFunction : public Node
    {
        DEFINE_NODE(SprFunction, nkSparrowDeclSprFunction, "Sparrow.Decl.Function");

    public:
        SprFunction(const Location& loc, string name, NodeList* parameters, Node* returnType, Node* body, Node* ifClause, AccessType accessType = publicAccess);

        bool hasThisParameters() const;

        Node* returnType() const;
        Node* body() const;

        Feather::Function* resultingFun() const;

        virtual string toString() const;
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();

    private:
        void handleStaticCtorDtor(bool ctor);
    };
}
