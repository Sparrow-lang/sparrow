#pragma once

#include <Nest/Intermediate/Type.h>
#include <Nest/Intermediate/EvalMode.h>

FWD_CLASS1(Feather, Class);

namespace Feather
{
    /// A type that has storage (data type or something similar)
    class StorageType : public Nest::Type
    {
    protected:
        StorageType(unsigned typeId, Nest::EvalMode mode);
        virtual ~StorageType() {}

    public:
        /// Getter for the class that introduces this data type - can be null
        virtual Class* classDecl() const;
        
        /// The number of references applied
        virtual uint8_t noReferences() const { return data_.numReferences; }

        /// If the class has an associated name this will return it; otherwise it returns nullptr
        const string* nativeName() const;
    };
}
