#pragma once

#include "Diagnostic.h"
#include "DiagnosticFormatter.hpp"
#include "Nest/Api/EvalMode.h"

// Used to de-optimize error reporting
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define __REP_IMPL_RET(retVal, type, fmt, loc)                                                     \
    return mkDiagReporterWithReturnFromFormatter(retVal) =                                         \
                   Nest::Common::DiagnosticFormatter(type, fmt, (loc))
#define __REP_IMPL(type, fmt, loc)                                                                 \
    DiagReporterFromFormatter() = Nest::Common::DiagnosticFormatter(type, fmt, (loc))

#define REP_INTERNAL(loc, fmt) __REP_IMPL(diagInternalError, fmt, (loc))
#define REP_ERROR(loc, fmt) __REP_IMPL(diagError, fmt, (loc))
#define REP_ERROR_RET(retVal, loc, fmt) __REP_IMPL_RET(retVal, diagError, fmt, (loc))
#define REP_WARNING(loc, fmt) __REP_IMPL(diagWarning, fmt, (loc))
#define REP_INFO(loc, fmt) __REP_IMPL(diagInfo, fmt, (loc))

;

// Stream output operators for the most common nest types
using Location = struct Nest_Location;
using Node = struct Nest_Node;
using TypeRef = const struct Nest_Type*;

ostream& operator<<(ostream& os, const Location* loc);
ostream& operator<<(ostream& os, const Location& loc);
ostream& operator<<(ostream& os, const Node* n);
ostream& operator<<(ostream& os, TypeRef t);
ostream& operator<<(ostream& os, EvalMode mode);

struct DiagReporterFromFormatter {
    DiagReporterFromFormatter& operator=(const Nest::Common::DiagnosticFormatter& fmt) {
        Nest_reportDiagnostic(fmt.location(), fmt.severity(), fmt.message().c_str());
        return *this;
    }
};

template <typename RetType> struct DiagReporterWithReturnFromFormatter {
    DiagReporterWithReturnFromFormatter(RetType retVal)
        : retVal_(retVal) {}

    RetType operator=(const Nest::Common::DiagnosticFormatter& fmt) { // NOLINT
        Nest_reportDiagnostic(fmt.location(), fmt.severity(), fmt.message().c_str());
        return retVal_;
    }

    RetType retVal_;
};

template <typename RetType>
DiagReporterWithReturnFromFormatter<RetType> mkDiagReporterWithReturnFromFormatter(
        const RetType& retVal) {
    return DiagReporterWithReturnFromFormatter<RetType>(retVal);
}
