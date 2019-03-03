#include "Nest/Utils/Profiling.h"

#if SPARROW_PROFILING

// Include TracyClient.cpp after defining TRACY_ENABLE
#include "../../../externals/tracy/TracyClient.cpp"

extern "C" const Nest_Profiling_LocType* Nest_Profiling_createLoc(
        const char* name, const char* function, const char* file, unsigned line, unsigned color) {
    Nest_Profiling_LocType* loc = (Nest_Profiling_LocType*)malloc(sizeof(Nest_Profiling_LocType));
    loc->name = name;
    loc->function = function;
    loc->file = file;
    loc->line = line;
    loc->color = color;
    return loc;
}

#endif