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
    if ( loc->startLineNo == loc->endLineNo )
        os << '(' << loc->startLineNo << ':' << loc->startColNo << '-' << loc->endColNo << ')';
    else
        os << '(' << loc->startLineNo << ':' << loc->startColNo << " - " << loc->endLineNo << ':' << loc->endColNo << ')';
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
        unsigned short startLineNo = obj.startLineNo;
        unsigned short endLineNo = obj.endLineNo;
        unsigned short startColNo = obj.startColNo;
        unsigned short endColNo = obj.endColNo;
        typedef unsigned long long u64;
        u64 locInfo = u64(endColNo) + (u64(startColNo)<<16) + (u64(endLineNo)<<32) + (u64(startLineNo)<<48);
        ar.write("locInfo", locInfo);
    }
    else
    {
        ar.write("startLineNo", obj.startLineNo);
        ar.write("endLineNo", obj.endLineNo);
        ar.write("startColNo", obj.startColNo);
        ar.write("endColNo", obj.endColNo);
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
        obj.endColNo = locInfo & 0xFFFF;
        obj.startColNo = (locInfo >> 16) & 0xFFFF;
        obj.startLineNo = (locInfo >> 32) & 0xFFFF;
        obj.endLineNo = (locInfo >> 48) & 0xFFFF;
    }
    else
    {
        ar.read("startLineNo", obj.startLineNo);
        ar.read("endLineNo", obj.endLineNo);
        ar.read("startColNo", obj.startColNo);
        ar.read("endColNo", obj.endColNo);
    }
}


