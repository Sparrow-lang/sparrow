#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_CompilerModule CompilerModule;

/// Getter for the Nest module
CompilerModule* getNestModule();

#ifdef __cplusplus
}
#endif
