#pragma once

#include <Feather/FeatherNodeCommonsH.h>
#include "CallConvention.h"

namespace Feather
{
    /// Interface class for all the declarations classes.
    class Function : public DynNode
    {
        DEFINE_NODE(Function, nkFeatherDeclFunction, "Feather.Decls.Function");

    public:
        Function(const Location& loc, string name, DynNode* resultType, DynNode* body, DynNodeVector parameters, CallConvention callConv = ccC);

        void addParameter(DynNode* parameter, bool first = false);
        void setResultType(DynNode* resultType);
        void setBody(DynNode* body);

        size_t numParameters() const;
        DynNode* getParameter(size_t idx) const;
        
        TypeRef resultType() const;

        DynNode* body() const;

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
