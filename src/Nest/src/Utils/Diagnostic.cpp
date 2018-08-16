#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/src/Utils/ConsoleColors.hpp"
#include "Nest/Api/StringRef.h"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Api/SourceCode.h"
#include "Nest/Api/Type.h"

#include <cstdarg>

static int _reportingEnabled = 1;
static int _numErrors = 0;
static int _numSupppresedErrors = 0;

using namespace Nest;

namespace {
void doReport(const Location& loc, Nest_DiagnosticSeverity severity, const string& message) {
    cerr << endl;

    // Write location: 'filename(line:col) : '
    if (!Nest_isLocEmpty(&loc)) {
        cerr << loc << " : ";
    }

    // Write the severity
    switch (severity) {
    case diagInternalError:
        cerr << ConsoleColors::fgHiMagenta << "INTERNAL ERROR : ";
        break;
    case diagError:
        cerr << ConsoleColors::fgLoRed << "ERROR : ";
        break;
    case diagWarning:
        cerr << ConsoleColors::fgHiYellow << "WARNING : ";
        break;
    case diagInfo:
    default:
        break;
    }

    // Write the diagnostic text
    cerr << ConsoleColors::stClear << ConsoleColors::stBold << message << ConsoleColors::stClear
         << endl;

    // Try to write the source line no in which the diagnostic occurred
    if (!Nest_isLocEmpty(&loc)) {
        // Get the actual source line
        StringRef sourceLine = Nest_getSourceCodeLine(loc.sourceCode, loc.start.line);
        size_t sourceLineLen = sourceLine.end - sourceLine.begin;
        if (sourceLineLen > 0) {
            char lastChar = *(sourceLine.end - 1);

            // Add the source line
            cerr << "> " << string(sourceLine.begin, sourceLine.end);
            if (lastChar != '\n')
                cerr << "\n";

            // Add the pointer to the output string
            int count = loc.end.line == loc.start.line ? loc.end.col - loc.start.col
                                                       : sourceLineLen - loc.start.col + 1;
            if (count <= 1)
                count = 1;
            cerr << "  ";
            cerr << string(loc.start.col - 1, ' '); // spaces used for alignment
            cerr << ConsoleColors::fgHiGreen;
            cerr << string(count, '\''); // underline the whole location range
            cerr << ConsoleColors::stClear << endl;
        }
    }
}

inline const char* _evalModeToString(EvalMode mode) {
    switch (mode) {
    case modeCt:
        return "ct";
    case modeRt:
        return "rt";
    default:
        return "unspecified";
    }
}
} // namespace

void Nest_reportDiagnostic(Location loc, Nest_DiagnosticSeverity severity, const char* message) {
    // If error reporting is paused, allow only internal errors
    if (severity != diagInternalError && !_reportingEnabled) {
        if (severity == diagError)
            ++_numSupppresedErrors;
        return;
    }

    if (severity == diagError)
        ++_numErrors;

    // Show the diagnostic
    doReport(loc, severity, message);

    if (severity == diagInternalError)
        exit(-1);
    // if ( severity == diagError )
    //     exit(-1);
}

void Nest_reportFmt(Location loc, Nest_DiagnosticSeverity severity, const char* fmt, ...) {
    static const int maxMessageLen = 1024;

    char buffer[maxMessageLen];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, maxMessageLen, fmt, args);
    Nest_reportDiagnostic(loc, severity, buffer);
    va_end(args);
}

void Nest_enableReporting(int enable) {
    if (!enable)
        _numSupppresedErrors = 0; // Reset the counter when disabling errors
    _reportingEnabled = enable;
}

int Nest_isReportingEnabled() { return _reportingEnabled; }

int Nest_getErrorsNum() { return _numErrors; }

int Nest_getSuppressedErrorsNum() { return _numSupppresedErrors; }

void Nest_resetDiagnostic() {
    _reportingEnabled = 1;
    _numErrors = 0;
    _numSupppresedErrors = 0;
}

ostream& operator<<(ostream& os, const Location* loc) {
    return os << *loc;
}
ostream& operator<<(ostream& os, const Location& loc) {
    os << (loc.sourceCode ? loc.sourceCode->url : "<no-source>");
    if (loc.start.line == loc.end.line)
        os << '(' << loc.start.line << ':' << loc.start.col << '-' << loc.end.col << ')';
    else
        os << '(' << loc.start.line << ':' << loc.start.col << " - " << loc.end.line << ':'
           << loc.end.col << ')';
    return os;
}
ostream& operator<<(ostream& os, Node* n) {
    return os << NodeHandle(n);
}
ostream& operator<<(ostream& os, TypeRef t) {
    if (t)
        os << t->description;
    else
        os << "<null-type>";
    return os;
}

ostream& operator<<(ostream& os, EvalMode mode) {
    os << _evalModeToString(mode);
    return os;
}
