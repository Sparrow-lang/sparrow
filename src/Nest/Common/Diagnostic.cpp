#include "StdInc.h"
#include "Diagnostic.hpp"
#include "DiagnosticReporter.h"


void Nest_reportDiagnostic(Location loc, DiagnosticSeverity severity, const char* message)
{
    Nest::Common::diagnosticReporter().report(severity, message, loc);

    if ( severity == diagInternalError )
        exit(-1);
}

void Nest_reportFmt(Location loc, DiagnosticSeverity severity, const char* fmt, ...)
{
    static const int maxMessageLen = 1024;

    char buffer[maxMessageLen];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, maxMessageLen, fmt, args);
    Nest_reportDiagnostic(loc, severity, buffer);
    va_end(args);
}
