#include <StdInc.h>
#include "DiagnosticReporter.h"
#include "CompilationError.h"
#include "ConsoleColors.h"
#include <Compiler.h>
#include <Common/Diagnostic.h>
#include <Frontend/SourceCode.h>
#include <Frontend/LocationSer.h>

using namespace Nest;
using namespace Nest::Common;

namespace
{
    void doReport(const Location& loc, DiagnosticSeverity severity, const string& message)
    {
        // Write location: 'filename(line:col) : '
        const SourceCode* sourceCode = nullptr;
        if ( !Nest_isLocEmpty(&loc) )
        {
            sourceCode = (const SourceCode*) loc.sourceCode;
            ASSERT(loc.sourceCode);
            cerr << loc << " : ";
        }

        // Write the severity
        switch ( severity )
        {
        case diagInternalError:     cerr << ConsoleColors::fgHiMagenta << "INTERNAL ERROR : "; break;
        case diagError:             cerr << ConsoleColors::fgLoRed << "ERROR : "; break;
        case diagWarning:           cerr << ConsoleColors::fgHiYellow << "WARNING : "; break;
        case diagInfo:
        default:                    break;
        }

        // Write the diagnostic text
        cerr << ConsoleColors::stClear << ConsoleColors::stBold << message << ConsoleColors::stClear << endl;

        // Try to write the source line no in which the diagnostic occurred
        if ( !Nest_isLocEmpty(&loc) )
        {
            // Get the actual source line
            const string& sourceLine = sourceCode->getSourceCodeLine(loc.start.line);
            if ( !sourceLine.empty() )
            {
                char lastChar = sourceLine[sourceLine.size()-1];
            
                // Add the source line
                cerr << "> " << sourceLine;
                if ( lastChar != '\n' )
                    cerr << "\n";

                // Add the pointer to the output string
                int count = loc.end.line == loc.start.line
                                ? loc.end.col - loc.start.col
                                : sourceLine.length() - loc.start.col+1;
                if ( count <= 1 )
                    count = 1;
                cerr << "  ";
                cerr << string(loc.start.col-1, ' ');      // spaces used for alignment
                cerr << ConsoleColors::fgLoRed;
                cerr << string(count, '~');                   // arrow to underline the whole location range
                cerr << ConsoleColors::stClear << endl;
            }
        }
    }
}

DiagnosticReporter::DiagnosticReporter()
    : errorsCount_(0)
    , warningsCount_(0)
    , severityLevel_(diagInfo)
{
}

void DiagnosticReporter::setSeverityLevel(DiagnosticSeverity level)
{
    severityLevel_ = level;
}

DiagnosticSeverity DiagnosticReporter::severityLevel() const
{
    return severityLevel_;
}

bool DiagnosticReporter::isEnabledFor(DiagnosticSeverity type) const
{
    return (int) severityLevel_ >= (int) type;
}

bool DiagnosticReporter::isDebugEnabled() const
{
    return (int) severityLevel_ >= (int) diagInfo;
}

bool DiagnosticReporter::wasError() const
{
    return errorsCount_ > 0;
}

int DiagnosticReporter::errorsCount() const
{
    return errorsCount_;
}

int DiagnosticReporter::warningsCount() const
{
    return warningsCount_;
}

void DiagnosticReporter::resetCounters()
{
    errorsCount_ = warningsCount_ = 0;
}


void DiagnosticReporter::report(DiagnosticSeverity severity, const string& message, bool dontThrow)
{
    report(severity, message, Location(), dontThrow);
}

void DiagnosticReporter::report(DiagnosticSeverity severity, const string& message, const Location& loc, bool dontThrow)
{
    if ( isEnabledFor(severity) )
    {
        if ( severity == diagInternalError || severity == diagError )
            ++errorsCount_;
        else if ( severity == diagWarning )
            ++warningsCount_;

        doReport(loc, severity, message);
    }

    if ( !dontThrow && (severity == diagInternalError || severity == diagError) )
        throw CompilationError(severity, message);
}

void DiagnosticReporter::operator=(const DiagnosticFormatter& fmt)
{
    report(fmt.severity(), fmt.message(), fmt.location(), fmt.dontThrow());
}

DiagnosticReporter& Nest::Common::diagnosticReporter()
{
    return theCompiler().diagnosticReporter();
}
