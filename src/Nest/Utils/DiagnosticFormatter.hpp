#pragma once

#include "Diagnostic.h"
#include "Nest/Api/Location.h"

namespace Nest { namespace Common
{
    /// Helper class that formats a diagnostic message
    /// Uses Boost::format to format the diagnostic message
    class DiagnosticFormatter
    {
    public:
        DiagnosticFormatter(DiagnosticSeverity severity, const char* fmt, bool dontThrow = false);
        DiagnosticFormatter(DiagnosticSeverity severity, const char* fmt, Location loc, bool dontThrow = false);

        /// Add a parameter to the diagnostic formatter
        template <typename T>
        DiagnosticFormatter& operator % (const T& obj)
        {
            fmt_ % obj;
            return *this;
        }


        DiagnosticSeverity severity() const { return severity_; }
        string message() const { return fmt_.str(); }
        const Location& location() const { return location_; }
        bool dontThrow() const { return dontThrow_; }

    private:
        DiagnosticSeverity severity_;   ///< The severity of the diagnostic to be reported
        boost::format fmt_;             ///< The formatter object
        Location location_;             ///< The location in the source code corresponding to this diagnostic
        bool dontThrow_;                ///< If set, we won't throw an exception
    };
}}
