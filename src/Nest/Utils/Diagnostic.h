#pragma once

#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Utils/Assert.h"
#include "Nest/Utils/LocationUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Location Nest_Location;

/// The severity type of a reported diagnostic message
enum Nest_DiagnosticSeverity {
    diagInternalError,
    diagError,
    diagWarning,
    diagInfo,
};
typedef enum Nest_DiagnosticSeverity Nest_DiagnosticSeverity;

/// Report a diagnostic of the given severity, with the given message
void Nest_reportDiagnostic(
        Nest_Location loc, Nest_DiagnosticSeverity severity, const char* message);

/// Report a diagnostic, version with string formatting
void Nest_reportFmt(Nest_Location loc, Nest_DiagnosticSeverity severity, const char* fmt, ...);

/// Enables/disables the error reporting
/// If disabled, no errors, warnings or info messages will be printed out
void Nest_enableReporting(int enable);

/// Checks if error reporting is enabled
int Nest_isReportingEnabled();

/// Returns the number of errors returned so far
int Nest_getErrorsNum();

/// Returned the number of errors that were generated while error reporting was disabled
int Nest_getSuppressedErrorsNum();

//! Resets the diagnostic reporting
void Nest_resetDiagnostic();

#ifdef __cplusplus
}
#endif

#define NOLOC Nest_mkEmptyLocation()

#ifndef CHECK
#define CHECK(loc, val)                                                                            \
    if (!!(val))                                                                                   \
        ;                                                                                          \
    else {                                                                                         \
        Nest_reportFmt(loc, diagInternalError,                                                     \
                "Non-zero expression expected for '%s' (file %s, line %d)", #val, __FILE__,        \
                __LINE__);                                                                         \
        ASSERT(false);                                                                             \
    }
#endif

#define EXPECT_KIND(loc, node, kind)                                                               \
    if (!(node))                                                                                   \
        Nest_reportFmt(loc, diagInternalError, "Expected non-NULL node: '%s' (file %s, line %d)",  \
                #node, __FILE__, __LINE__);                                                        \
    else if ((node)->nodeKind != (kind)) {                                                         \
        Nest_reportFmt(loc, diagInternalError,                                                     \
                "Node: '%s' of kind %s needs to be of kind %s; (file %s, line %d)", #node,         \
                Nest_getNodeKindName((node)->nodeKind), Nest_getNodeKindName(kind), __FILE__,      \
                __LINE__);                                                                         \
        ASSERT(false);                                                                             \
    }
