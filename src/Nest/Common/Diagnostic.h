#pragma once

#include "DiagnosticFormatter.h"
#include "DiagnosticReporter.h"
#include "CompilationError.h"

#define __REP_IMPL(type, fmt, loc, dontThrow) \
    Nest::Common::diagnosticReporter() = Nest::Common::DiagnosticFormatter(Nest::Common:: type, fmt, (loc), (dontThrow))

#define __REP_IMPL_NL(type, fmt, dontThrow) \
    Nest::Common::diagnosticReporter() = Nest::Common::DiagnosticFormatter(Nest::Common:: type, fmt, (dontThrow))


#define NOLOC                           mkEmptyLocation()

#define REP_INTERNAL(loc, fmt)          __REP_IMPL(diagInternalError, fmt, (loc), false)
#define REP_ERROR(loc, fmt)             __REP_IMPL(diagError, fmt, (loc), false)
#define REP_WARNING(loc, fmt)           __REP_IMPL(diagWarning, fmt, (loc), false)
#define REP_INFO(loc, fmt)              __REP_IMPL(diagInfo, fmt, (loc), false)

#define REP_ERROR_NOTHROW(loc, fmt)     __REP_IMPL(diagError, fmt, (loc), true)
#define REP_ERROR_THROW(msg)            throw Nest::Common::CompilationError(Nest::Common::diagError, msg)


#define CHECK(loc, val) \
    if ( !!(val) ); else REP_INTERNAL(loc, "Non-zero expression expected for '%1%', file %2%, line %3%") % #val % __FILE__ % __LINE__;

#define REQUIRE_NODE(loc, node) \
    if ( node ) ; else \
        REP_INTERNAL((loc), "Expected AST node (%1%)") % ( #node )

#define REQUIRE_TYPE(loc, type) \
    if ( type ) ; else \
        REP_INTERNAL((loc), "Expected type (%1%)") % ( #type )
