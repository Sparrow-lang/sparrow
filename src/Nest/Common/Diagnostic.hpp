#pragma once

#include "Diagnostic.h"
#include "DiagnosticFormatter.h"
#include "CompilationError.h"

#define __REP_IMPL(type, fmt, loc, dontThrow) \
    DiagReporterFromFormatter() = Nest::Common::DiagnosticFormatter(type, fmt, (loc), (dontThrow))

#define REP_INTERNAL(loc, fmt)          __REP_IMPL(diagInternalError, fmt, (loc), false)
#define REP_ERROR(loc, fmt)             __REP_IMPL(diagError, fmt, (loc), false)
#define REP_WARNING(loc, fmt)           __REP_IMPL(diagWarning, fmt, (loc), false)
#define REP_INFO(loc, fmt)              __REP_IMPL(diagInfo, fmt, (loc), false)

#define REP_ERROR_NOTHROW(loc, fmt)     __REP_IMPL(diagError, fmt, (loc), true)
#define REP_ERROR_THROW(msg)            throw Nest::Common::CompilationError(diagError, msg)

struct DiagReporterFromFormatter
{
    void operator=(const Nest::Common::DiagnosticFormatter& fmt)
    {
        Nest_reportDiagnostic(fmt.location(), fmt.severity(), fmt.message().c_str());
    }

};
