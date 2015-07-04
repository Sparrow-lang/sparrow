#pragma once

FWD_STRUCT1(Nest, TypeData)

namespace Nest
{
    /// A type reference -- this is guaranteed to be unique for each type
    typedef struct TypeData* TypeRef;
}