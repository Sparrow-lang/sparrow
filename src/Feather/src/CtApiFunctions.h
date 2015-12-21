#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Backend Backend;

void Feather_registerCtApiFunctions(Backend* backend);

#ifdef __cplusplus
}
#endif
