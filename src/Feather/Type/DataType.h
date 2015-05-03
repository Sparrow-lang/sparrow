#pragma once

#include "StorageType.h"

namespace Feather
{
    /// A type that contains data
    /// A data type is introduced by a class definition and can have one or more references; a data type must have a size
    class DataType : public StorageType
    {
    public:
        static DataType* get(Class* classDecl, uint8_t noReferences = 0, Nest::EvalMode mode = Nest::modeRtCt);

        /// Getter for the class that introduces this data type
        virtual Class* classDecl() const;
        
        /// The number of references applied
        virtual uint8_t noReferences() const;

    public:
        virtual string toString() const;
        virtual bool hasStorage() const { return true; }
        virtual bool canBeUsedAtRt() const;
        virtual bool canBeUsedAtCt() const;
        virtual DataType* changeMode(Nest::EvalMode newMode);

    private:
        DataType(Class* classDecl, uint8_t noReferences, Nest::EvalMode mode);
    };
}
