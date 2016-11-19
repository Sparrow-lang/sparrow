#pragma once

#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Utils/Assert.h"
#include "Nest/Utils/LocationUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Location Location;

/// The severity type of a reported diagnostic message
enum Nest_DiagnosticSeverity
{
    diagInternalError,
    diagError,
    diagWarning,
    diagInfo,
};
typedef enum Nest_DiagnosticSeverity Nest_DiagnosticSeverity;
typedef enum Nest_DiagnosticSeverity DiagnosticSeverity;

/// Report a diagnostic of the given severity, with the given message
void Nest_reportDiagnostic(Location loc, DiagnosticSeverity severity, const char* message);

/// Report a diagnostic, version with string formatting
void Nest_reportFmt(Location loc, DiagnosticSeverity severity, const char* fmt, ...);

/// Enables/disables the error reporting
/// If disabled, no errors, warnings or info messages will be printed out
void Nest_enableReporting(int enable);

/// Checks if error reporting is enabled
int Nest_isReportingEnabled();

/// Returns the number of errors returned so far
int Nest_getErrorsNum();

/// Returned the number of errors that were generated while error reporting was disabled
int Nest_getSuppressedErrorsNum();


#ifdef __cplusplus
}
#endif

#define NOLOC                           Nest_mkEmptyLocation()

#define CHECK(loc, val) \
    if ( !!(val) ); else \
        Nest_reportFmt(loc, diagInternalError, "Non-zero expression expected for '%s' (file %s, line %d)", #val, __FILE__, __LINE__)

#define EXPECT_KIND(loc, node, kind) \
    if ( !(node) ) \
        Nest_reportFmt(loc, diagInternalError, "Expected non-NULL node: '%s' (file %s, line %d)", #node, __FILE__, __LINE__); \
    else if ( (node)->nodeKind != (kind) ) \
        Nest_reportFmt(loc, diagInternalError, "Node: '%s' of kind %s needs to be of kind %s; (file %s, line %d)", \
            #node, Nest_getNodeKindName((node)->nodeKind), Nest_getNodeKindName(kind), __FILE__, __LINE__)


