#include <StdInc.h>
#include "Location.h"
#include "SourceCode.h"
#include <Common/Ser/OutArchive.h>
#include <Common/Ser/InArchive.h>

#include <boost/lexical_cast.hpp>

using namespace Nest;

Location::Location()
    : sourceCode_(nullptr)
    , startLineNo_(1)
    , startColNo_(1)
    , endLineNo_(1)
    , endColNo_(1)
{
}

Location::Location(const SourceCode& sourceCode)
    : sourceCode_(&sourceCode)
    , startLineNo_(1)
    , startColNo_(1)
    , endLineNo_(1)
    , endColNo_(1)
{
}

Location::Location(const SourceCode& sourceCode, size_t lineNo, size_t colNo)
    : sourceCode_(&sourceCode)
    , startLineNo_(lineNo)
    , startColNo_(colNo)
    , endLineNo_(lineNo)
    , endColNo_(colNo)
{
}

Location::Location(const SourceCode& sourceCode, size_t startLineNo, size_t startColNo, size_t endLineNo, size_t endColNo)
    : sourceCode_(&sourceCode)
    , startLineNo_(startLineNo)
    , startColNo_(startColNo)
    , endLineNo_(endLineNo)
    , endColNo_(endColNo)
{
}

bool Location::empty() const
{
    return nullptr == sourceCode_;
}

const SourceCode* Location::sourceCode() const
{
    return sourceCode_;
}

size_t Location::startLineNo() const
{
    return startLineNo_;
}

size_t Location::startColNo() const
{
    return startColNo_;
}


size_t Location::endLineNo() const
{
    return endLineNo_;
}

size_t Location::endColNo() const
{
    return endColNo_;
}

string Location::toString() const
{
    ostringstream oss;
    oss << *this;
    return oss.str();
}

string Location::getCorrespondingCode() const
{
    if ( sourceCode_ )
    {
        string line = sourceCode_->getSourceCodeLine(startLineNo_);
        size_t count = endLineNo_ == startLineNo_ ? endColNo_-startColNo_ : line.size()-startColNo_;
        return line.substr(startColNo_-1, count);
    }
    else
        return string();
}

void Location::step()
{
    startColNo_ = endColNo_;
    startLineNo_ = endLineNo_;
}

void Location::addColumns(size_t count)
{
    endColNo_ += count;
}

void Location::addLines(size_t count)
{
    endLineNo_ += count;
    endColNo_ = 1;
}

void Location::copyStart(const Location& rhs)
{
    sourceCode_ = rhs.sourceCode_;
    startLineNo_ = rhs.startLineNo_;
    startColNo_ = rhs.startColNo_;
}

void Location::copyEnd(const Location& rhs)
{
    endLineNo_ = rhs.endLineNo_;
    endColNo_ = rhs.endColNo_;
}

void Location::setAsStartOf(const Location& rhs)
{
    sourceCode_ = rhs.sourceCode_;
    endLineNo_ = startLineNo_ = rhs.startLineNo_;
    endColNo_ = startColNo_ = rhs.startColNo_;
}

void Location::setAsEndOf(const Location& rhs)
{
    sourceCode_ = rhs.sourceCode_;
    endLineNo_ = startLineNo_ = rhs.endLineNo_;
    endColNo_ = startColNo_ = rhs.endColNo_;
}


void Location::save(Common::Ser::OutArchive& ar) const
{
    using namespace Common::Ser;

    // TODO: source-code
    if ( true )
    {
        // Use compact location info
        unsigned short startLineNo = startLineNo_;
        unsigned short endLineNo = endLineNo_;
        unsigned short startColNo = startColNo_;
        unsigned short endColNo = endColNo_;
        typedef unsigned long long u64;
        u64 locInfo = u64(endColNo) + (u64(startColNo)<<16) + (u64(endLineNo)<<32) + (u64(startLineNo)<<48);
        ar.write("locInfo", locInfo);
    }
    else
    {
        ar.write("startLineNo", startLineNo_);
        ar.write("endLineNo", endLineNo_);
        ar.write("startColNo", startColNo_);
        ar.write("endColNo", endColNo_);
    }
}

void Location::load(Common::Ser::InArchive& ar)
{
    // TODO: source-code
    if ( true )
    {
        typedef unsigned long long u64;
        u64 locInfo;
        ar.read("locInfo", locInfo);
        endColNo_ = locInfo & 0xFFFF;
        startColNo_ = (locInfo >> 16) & 0xFFFF;
        startLineNo_ = (locInfo >> 32) & 0xFFFF;
        endLineNo_ = (locInfo >> 48) & 0xFFFF;
    }
    else
    {
        ar.read("startLineNo", startLineNo_);
        ar.read("endLineNo", endLineNo_);
        ar.read("startColNo", startColNo_);
        ar.read("endColNo", endColNo_);
    }
}

ostream& Nest::operator << (ostream& os, const Location& loc)
{
    os << (loc.sourceCode() ? loc.sourceCode()->filename() : string("<no-source>"));
    if ( loc.startLineNo() == loc.endLineNo() )
        os << '(' << loc.startLineNo() << ':' << loc.startColNo() << '-' << loc.endColNo() << ')';
    else
        os << '(' << loc.startLineNo() << ':' << loc.startColNo() << " - " << loc.endLineNo() << ':' << loc.endColNo() << ')';
    return os;
}

bool Nest::operator <(const Location& loc1, const Location& loc2)
{
    if ( loc1.sourceCode() != loc2.sourceCode() )
        return false;
    if ( loc1.startLineNo() < loc2.startLineNo() )
        return true;
    if ( loc1.startLineNo() > loc2.startLineNo() )
        return false;
    if ( loc1.startColNo() < loc2.startColNo() )
        return true;
    if ( loc1.startColNo() > loc2.startColNo() )
        return false;
    if ( loc1.endLineNo() < loc2.endLineNo() )
        return true;
    if ( loc1.endLineNo() > loc2.endLineNo() )
        return false;
    if ( loc1.endColNo() < loc2.endColNo() )
        return true;
    if ( loc1.endColNo() > loc2.endColNo() )
        return false;
    return false;
}

bool Nest::operator ==(const Location& l1, const Location& l2)
{
    return l1.sourceCode() == l2.sourceCode()
        && l1.startLineNo() == l2.startLineNo()
        && l1.startColNo() == l2.startColNo()
        && l1.endLineNo() == l2.endLineNo()
        && l1.endColNo() == l2.endColNo()
        ;
}
bool Nest::operator !=(const Location& l1, const Location& l2)
{
    return !(l1 == l2);
}

