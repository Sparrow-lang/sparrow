#pragma once

#include <Feather/FeatherNodeCommonsH.h>
#include "CallConvention.h"

namespace Feather
{
    /// Interface class for all the declarations classes.
    class Function : public Node
    {
        DEFINE_NODE(Function, nkFeatherDeclFunction, "Feather.Decls.Function");

    public:
        Function(const Location& loc, string name, Node* resultType, Node* body, NodeVector parameters, CallConvention callConv = ccC);

        void addParameter(Node* parameter, bool first = false);
        void setResultType(Node* resultType);
        void setBody(Node* body);

        size_t numParameters() const;
        Node* getParameter(size_t idx) const;
        
        TypeRef resultType() const;

        Node* body() const;

        CallConvention callConvention() const;

    public:
        virtual void dump(ostream& os) const;
        virtual string toString() const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
