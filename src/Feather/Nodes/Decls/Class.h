#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// DynNode that represents a class (data type) declaration
    /// A class has a name an can have zero or more fields declared inside it.
    /// The type for a Class is always a DataType
    class Class : public DynNode
    {
        DEFINE_NODE(Class, nkFeatherDeclClass, "Feather.Decls.Class");

    public:
        Class(const Location& loc, string name, DynNodeVector fields);

        void addFields(const DynNodeVector& fields);

        const DynNodeVector& fields() const;

    public:
        void dump(ostream& os) const;

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();
    };
}
