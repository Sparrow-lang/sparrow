#pragma once

#include <Nest/Intermediate/Type.h>

namespace Feather
{
    using namespace Nest;

    /// The Void type
    /// This is a non-storage, special type, to indicate the absence of any information
    class Void : public Nest::Type
    {
    public:
        /// Getter for the instance of the Void type
        static Void* get(EvalMode mode);
    };
}
