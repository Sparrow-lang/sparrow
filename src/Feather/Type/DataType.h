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

    public:
        virtual string toString() const;
        virtual DataType* changeMode(Nest::EvalMode newMode);

    private:
        DataType(Class* classDecl, uint8_t noReferences, Nest::EvalMode mode);
    };
}
