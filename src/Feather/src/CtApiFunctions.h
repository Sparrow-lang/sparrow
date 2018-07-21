#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Backend Nest_Backend;

void Feather_registerCtApiFunctions(Nest_Backend* backend);

#ifdef __cplusplus
}
#endif
