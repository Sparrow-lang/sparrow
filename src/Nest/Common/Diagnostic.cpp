#include "StdInc.h"
#include "Diagnostic.hpp"
#include "DiagnosticReporter.h"

static int _reportingEnabled = 1;
static int _numErrors = 0;
static int _numSupppresedErrors = 0;

void Nest_reportDiagnostic(Location loc, DiagnosticSeverity severity, const char* message)
{
    // If error reporting is paused, allow only internal errors
    if ( severity != diagInternalError && !_reportingEnabled )
    {
        if ( severity == diagError )
            ++_numSupppresedErrors;
        return;
    }

    if ( severity == diagError )
        ++_numErrors;

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

void Nest_enableReporting(int enable)
{
    if ( !enable )
        _numSupppresedErrors = 0;   // Reset the counter when disabling errors
    _reportingEnabled = enable;
}

int Nest_isReportingEnabled()
{
    return _reportingEnabled;
}

int Nest_getErrorsNum()
{
    return _numErrors;
}

int Nest_getSuppressedErrorsNum()
{
    return _numSupppresedErrors;
}
