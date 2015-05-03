#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Node that represents a class (data type) declaration
    /// A class has a name an can have zero or more fields declared inside it.
    /// The type for a Class is always a DataType
    class Class : public Node
    {
        DEFINE_NODE(Class, nkFeatherDeclClass, "Feather.Decls.Class");

    public:
        Class(const Location& loc, string name, NodeVector fields);

        void addFields(const NodeVector& fields);

        const NodeVector& fields() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
