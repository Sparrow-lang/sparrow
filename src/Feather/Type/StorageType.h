#pragma once

#include <Nest/Intermediate/Type.h>
#include <Nest/Intermediate/EvalMode.h>

FWD_CLASS1(Feather, Class);

namespace Feather
{
    /// A type that has storage (data type or something similar)
    class StorageType : public Nest::Type
    {
    public:
        /// Getter for the class that introduces this data type - can be null
        Class* classDecl() const;
        
        /// The number of references applied
        uint8_t noReferences() const { return data_->numReferences; }

        /// If the class has an associated name this will return it; otherwise it returns nullptr
        const string* nativeName() const;
    };
}
