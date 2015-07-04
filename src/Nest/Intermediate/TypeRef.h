#pragma once

FWD_STRUCT1(Nest, Type)

namespace Nest
{
    /// A type reference -- this is guaranteed to be unique for each type
    typedef const struct TypeData* TypeRef;
}