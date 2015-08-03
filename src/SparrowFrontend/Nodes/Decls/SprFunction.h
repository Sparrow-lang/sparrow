#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Class that represents a Sparrow namespace declaration (or compilation unit)
    class SprFunction : public DynNode
    {
        DEFINE_NODE(SprFunction, nkSparrowDeclSprFunction, "Sparrow.Decl.Function");

    public:
        SprFunction(const Location& loc, string name, Node* parameters, DynNode* returnType, DynNode* body, DynNode* ifClause, AccessType accessType = publicAccess);

        bool hasThisParameters() const;

        DynNode* returnType() const;
        DynNode* body() const;

        Node* resultingFun() const;

        string toString() const;
        void dump(ostream& os) const;

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();

    private:
        void handleStaticCtorDtor(bool ctor);
    };
}
