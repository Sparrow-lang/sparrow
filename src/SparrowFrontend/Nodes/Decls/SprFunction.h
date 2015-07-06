#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>

FWD_CLASS1(Feather, Function);

namespace SprFrontend
{
    /// Class that represents a Sparrow namespace declaration (or compilation unit)
    class SprFunction : public DynNode
    {
        DEFINE_NODE(SprFunction, nkSparrowDeclSprFunction, "Sparrow.Decl.Function");

    public:
        SprFunction(const Location& loc, string name, NodeList* parameters, DynNode* returnType, DynNode* body, DynNode* ifClause, AccessType accessType = publicAccess);

        bool hasThisParameters() const;

        DynNode* returnType() const;
        DynNode* body() const;

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
