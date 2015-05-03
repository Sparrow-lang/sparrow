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

        /// Getter for the class that introduces this data type
        virtual Class* classDecl() const;
        
        /// The number of references applied
        virtual uint8_t noReferences() const;

    public:
        virtual string toString() const;
        virtual bool hasStorage() const { return true; }
        virtual bool canBeUsedAtRt() const;
        virtual bool canBeUsedAtCt() const;
        virtual ArrayType* changeMode(Nest::EvalMode newMode);

    private:
        ArrayType(StorageType* unitType, uint32_t count);
    };
}
