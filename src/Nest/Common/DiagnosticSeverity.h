#pragma once

namespace Nest { namespace Common
{
    /// The severity type of a reported diagnostic message
    enum DiagnosticSeverity
    {
        diagInternalError,
        diagError,
        diagWarning,
        diagInfo,
    };
}}
