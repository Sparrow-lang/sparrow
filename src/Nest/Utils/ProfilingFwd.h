#pragma once

#ifndef SPARROW_PROFILING
#define SPARROW_PROFILING 0
#endif

#if SPARROW_PROFILING

//! Profiling type to store the source location data (from the compiler).
//! Includes zone name, function name, filename, line number and color
typedef struct ___tracy_source_location_data Nest_Profiling_LocType;

#endif