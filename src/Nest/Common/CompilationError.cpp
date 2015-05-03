#include <StdInc.h>
#include "CompilationError.h"

using namespace Nest;
using namespace Nest::Common;

CompilationError::CompilationError(DiagnosticSeverity severity, const string& message)
    : runtime_error(message)
    , severity_(severity)
    , message_(message)
{
}

DiagnosticSeverity CompilationError::severity() const
{
    return severity_;
}

const string& CompilationError::message() const
{
    return message_;
}
