#pragma once

#include "DiagnosticSeverity.h"

typedef struct Location_t Location;

namespace Nest { namespace Common
{
    class DiagnosticFormatter;
    
    /// Class that is used to do diagnostic reporting for the compiler
    /// All the modules should use this class for error reporting
    class DiagnosticReporter
    {
    public:
        DiagnosticReporter();

        /// Sets the severity level for the diagnostic reporter
        void setSeverityLevel(DiagnosticSeverity level = diagInfo);
        
        /// Gets the current diagnostic severity level
        DiagnosticSeverity severityLevel() const;

        /// Check if the given severity will be handled (the reporter is enabled for the given severity)
        bool isEnabledFor(DiagnosticSeverity type) const;

        /// True if the debug reporting is enabled
        bool isDebugEnabled() const;

        /// Return true if there was an error
        bool wasError() const;

        /// Returns the number of errors issued so far
        int errorsCount() const;

        /// Returns the number of warnings issued so far
        int warningsCount() const;

        /// Reset the counters for errors and warnings
        void resetCounters();


        /// Report a diagnostic message
        void report(DiagnosticSeverity severity, const string& message, bool dontThrow = false);

        /// Report a diagnostic message at a certain source code location
        void report(DiagnosticSeverity severity, const string& message, const Location& loc, bool dontThrow = false);
        
        /// Report a formatted diagnostic
        void operator = (const DiagnosticFormatter& fmt);

    private:
        /// Number of errors reported so far
        int errorsCount_;
        /// Number of warnings reported so far
        int warningsCount_;

        /// The severity level the reporter; all the diagnostic message above this level will be ignored
        DiagnosticSeverity severityLevel_;
    };
    
    /// Getter for the diagnostic reporter of the compier
    DiagnosticReporter& diagnosticReporter();
}}
