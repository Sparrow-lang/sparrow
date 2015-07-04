#pragma once

#include "StorageType.h"

namespace Feather
{
    /// A type that holds N instances of a given storage type
    class ArrayType : public StorageType
    {
    public:
        static ArrayType* get(StorageType* unitType, uint32_t count);

        /// Getter for the unit type
        StorageType* unitType() const;

        /// Getter for the number of units in the buffer type
        uint32_t count() const;
    };
}
