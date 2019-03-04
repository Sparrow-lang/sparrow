#pragma once

#include "ProfilingFwd.h"

#if SPARROW_PROFILING

#define TRACY_ENABLE 1

#ifdef __cplusplus

////////////////////////////////////////////////////////////////////////////////
// C++ profiling definitions

#include "../../../externals/tracy/Tracy.hpp"

#include "Nest/Api/StringRef.h"

#define PROFILING_ZONE() ZoneScoped
#define PROFILING_ZONE_NAMED(staticName) ZoneScopedN(staticName)
#define PROFILING_ZONE_TEXT(text)                                                                  \
    ZoneScoped;                                                                                    \
    _Nest_Profiling_zoneSetText(___tracy_scoped_zone, text)
#define PROFILING_ZONE_NAMED_TEXT(staticName, text)                                                \
    ZoneScopedN(staticName);                                                                       \
    _Nest_Profiling_zoneSetText(___tracy_scoped_zone, text)
#define PROFILING_ZONE_SETTEEXT(text) _Nest_Profiling_zoneSetText(___tracy_scoped_zone, text)

#define PROFILING_PLOT(name, val) tracy::Profiler::PlotData(name, val)

#define PROFILING_MESSAGE_STATIC(staticText) tracy::Profiler::Message(staticText)
#define PROFILING_MESSAGE(text) _Nest_Profiling_message(text)

inline void _Nest_Profiling_zoneSetText(tracy::ScopedZone& zone, const char* text) {
    zone.Text(text, strlen(text));
}
inline void _Nest_Profiling_zoneSetText(tracy::ScopedZone& zone, Nest_StringRef text) {
    zone.Text(text.begin, text.end - text.begin);
}

inline void _Nest_Profiling_message(const char* text) {
    tracy::Profiler::Message(text, strlen(text));
}
inline void _Nest_Profiling_message(Nest_StringRef text) {
    tracy::Profiler::Message(text.begin, text.end - text.begin);
}

#else // __cplusplus

////////////////////////////////////////////////////////////////////////////////
// C-only profiling definitions

#include "../../../externals/tracy/TracyC.h"

typedef TracyCZoneCtx Nest_Profiling_ZoneCtx;

#define PROFILING_C_ZONE_BEGIN(ctx)                                                                \
    static const struct Nest_Profiling_LocType TracyConcat(__tracy_source_location, __LINE__) = {  \
            NULL, __FUNCTION__, __FILE__, (uint32_t)__LINE__, 0};                                  \
    Nest_Profiling_ZoneCtx ctx =                                                                   \
            ___tracy_emit_zone_begin(&TracyConcat(__tracy_source_location, __LINE__), 1);

#define PROFILING_C_ZONE_BEGIN_LOC(ctx, locPtr)                                                    \
    Nest_Profiling_ZoneCtx ctx = ___tracy_emit_zone_begin(locPtr, 1)

#define PROFILING_C_ZONE_END(ctx) ___tracy_emit_zone_end(ctx)
#define PROFILING_C_ZONE_SETTEEXT(ctx, text) ___tracy_emit_zone_text(ctx, text, strlen(text))

#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////
// Both C and C++ profiling definitions

#ifdef __cplusplus
extern "C" {
#endif

//! Creates a location type for profiling
const Nest_Profiling_LocType* Nest_Profiling_createLoc(
        const char* name, const char* function, const char* file, unsigned line, unsigned color);

#ifdef __cplusplus
}
#endif

#else // SPARROW_PROFILING

////////////////////////////////////////////////////////////////////////////////
// Profiling not enabled

#define PROFILING_ZONE()                            /*nothing*/
#define PROFILING_ZONE_NAMED(staticName)            /*nothing*/
#define PROFILING_ZONE_TEXT(text)                   /*nothing*/
#define PROFILING_ZONE_NAMED_TEXT(staticName, text) /*nothing*/
#define PROFILING_ZONE_SETTEEXT(text)               /*nothing*/
#define PROFILING_PLOT(name, val)                   /*nothing*/
#define PROFILING_MESSAGE_STATIC(staticText)        /*nothing*/
#define PROFILING_MESSAGE(text)                     /*nothing*/

#define PROFILING_C_ZONE_BEGIN(ctx)             /*nothing*/
#define PROFILING_C_ZONE_BEGIN_LOC(ctx, locPtr) /*nothing*/
#define PROFILING_C_ZONE_END(ctx)               /*nothing*/
#define PROFILING_C_ZONE_SETTEEXT(ctx, text)    /*nothing*/

#endif