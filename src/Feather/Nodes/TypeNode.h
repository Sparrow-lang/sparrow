#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that doesn't do anything.
    /// Used for placeholder
    class TypeNode : public DynNode
    {
        DEFINE_NODE(TypeNode, nkFeatherTypeNode, "Feather.TypeNode");

    public:
        TypeNode(const Location& loc, TypeRef type);

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
