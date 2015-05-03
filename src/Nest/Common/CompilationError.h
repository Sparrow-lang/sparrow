#pragma once

#include "DiagnosticSeverity.h"

#include <stdexcept>

namespace Nest { namespace Common
{
    /// Exception class that is thrown when a compilation error is encountered
    class CompilationError : public runtime_error
    {
    public:
        CompilationError(DiagnosticSeverity severity, const string& message);

        DiagnosticSeverity severity() const;
        const string& message() const;

    private:
        DiagnosticSeverity severity_;
        string message_;

    };
}}
