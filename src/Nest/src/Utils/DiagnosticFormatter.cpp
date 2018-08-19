#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/DiagnosticFormatter.hpp"

using namespace Nest;

DiagnosticFormatter::DiagnosticFormatter(
        Nest_DiagnosticSeverity severity, const char* fmt, bool dontThrow)
    : severity_(severity)
    , fmt_(fmt)
    , location_()
    , dontThrow_(dontThrow) {}

DiagnosticFormatter::DiagnosticFormatter(
        Nest_DiagnosticSeverity severity, const char* fmt, Location loc, bool dontThrow)
    : severity_(severity)
    , fmt_(fmt)
    , location_(loc)
    , dontThrow_(dontThrow) {}
