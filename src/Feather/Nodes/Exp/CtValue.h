#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a compile-time value
    class CtValue : public DynNode
    {
        DEFINE_NODE(CtValue, nkFeatherExpCtValue, "Feather.Exp.CtValue");

    public:
        CtValue(const Location& loc, TypeRef type, string data = string());

        /// Getter for the value type
        TypeRef valueType() const;

        /// Getter for the value data -- the data is encoded in a string
        const string& valueData() const;

        /// Getter for the value memory buffer of this value
        template <typename T>
        T* value() const
        {
            return (T*) (void*) valueData().c_str();
        }

        /// Returns true if this CT value is the same as the other one
        bool operator ==(const CtValue& other) const;
        bool operator !=(const CtValue& other) const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
