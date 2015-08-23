#pragma once

typedef struct Nest_Backend Backend;

namespace Feather
{
    void registerCtApiFunctions(Backend* backend);
}
