#include <StdInc.h>
#include "LocationSer.h"
#include "SourceCode.h"
#include <Common/Ser/OutArchive.h>
#include <Common/Ser/InArchive.h>

using namespace Nest;

ostream& operator << (ostream& os, const Location* loc)
{
    const SourceCode* sourceCode = (SourceCode*) loc->sourceCode;
    os << (sourceCode ? sourceCode->filename() : string("<no-source>"));
    if ( loc->start.line == loc->end.line )
        os << '(' << loc->start.line << ':' << loc->start.col << '-' << loc->end.col << ')';
    else
        os << '(' << loc->start.line << ':' << loc->start.col << " - " << loc->end.line << ':' << loc->end.col << ')';
    return os;
}
ostream& operator << (ostream& os, const Location& loc)
{
    return os << &loc;
}

void save(const Location& obj, Common::Ser::OutArchive& ar)
{
    using namespace Common::Ser;

    // TODO: source-code
    if ( true )
    {
        // Use compact location info
        unsigned short startLine = obj.start.line;
        unsigned short endLine = obj.end.line;
        unsigned short startCol = obj.start.col;
        unsigned short endCol = obj.end.col;
        typedef unsigned long long u64;
        u64 locInfo = u64(endCol) + (u64(startCol)<<16) + (u64(endLine)<<32) + (u64(startLine)<<48);
        ar.write("locInfo", locInfo);
    }
    else
    {
        ar.write("start.line", obj.start.line);
        ar.write("end.line", obj.end.line);
        ar.write("start.col", obj.start.col);
        ar.write("end.col", obj.end.col);
    }
}

void load(Location& obj, Common::Ser::InArchive& ar)
{
    // TODO: source-code
    if ( true )
    {
        typedef unsigned long long u64;
        u64 locInfo;
        ar.read("locInfo", locInfo);
        obj.end.col = locInfo & 0xFFFF;
        obj.start.col = (locInfo >> 16) & 0xFFFF;
        obj.start.line = (locInfo >> 32) & 0xFFFF;
        obj.end.line = (locInfo >> 48) & 0xFFFF;
    }
    else
    {
        ar.read("start.line", obj.start.line);
        ar.read("end.line", obj.end.line);
        ar.read("start.col", obj.start.col);
        ar.read("end.col", obj.end.col);
    }
}


