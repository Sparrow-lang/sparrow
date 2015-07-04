#pragma once

#include "StorageType.h"

namespace Feather
{
    /// A type that contains data
    /// A data type is introduced by a class definition and can have one or more references; a data type must have a size
    class LValueType : public StorageType
    {
    public:
        static LValueType* get(Type* base);

        /// Returns the base type of this type
        StorageType* baseType() const;
        
    public:
        virtual string toString() const;
        virtual LValueType* changeMode(Nest::EvalMode newMode);

    private:
        LValueType(Type* baseType);
    };
}
