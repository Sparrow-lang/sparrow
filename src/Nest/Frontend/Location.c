#include "Location.h"
#include "Assert.h"

#include <stdlib.h>

Location mkEmptyLocation()
{
    Location loc = { NULL, 1, 1, 1, 1 };
    return loc;
}
Location mkLocation(const void* sourceCode, unsigned int startLineNo, unsigned int startColNo, unsigned int endLineNo, unsigned int endColNo)
{
    Location loc = { sourceCode, startLineNo, startColNo, endLineNo, endColNo };
    return loc;
}
Location mkLocation1(const void* sourceCode, unsigned int lineNo, unsigned int colNo)
{
    Location loc = { sourceCode, lineNo, colNo, lineNo, colNo };
    return loc;
}

int isEmpty(const Location* loc)
{
    return loc->sourceCode == NULL;
}

void step(Location* loc)
{
    loc->startColNo = loc->endColNo;
    loc->startLineNo = loc->endLineNo;
}

void addColumns(Location* loc, unsigned int count)
{
    loc->endColNo += count;
}

void addLines(Location* loc, unsigned int count)
{
    loc->endLineNo += count;
    loc->endColNo = 1;
}

void copyStart(Location* loc, const Location* rhs)
{
    loc->sourceCode = rhs->sourceCode;
    loc->startLineNo = rhs->startLineNo;
    loc->startColNo = rhs->startColNo;
}

void copyEnd(Location* loc, const Location* rhs)
{
    loc->endLineNo = rhs->endLineNo;
    loc->endColNo = rhs->endColNo;
}

void setAsStartOf(Location* loc, const Location* rhs)
{
    loc->sourceCode = rhs->sourceCode;
    loc->endLineNo = loc->startLineNo = rhs->startLineNo;
    loc->endColNo = loc->startColNo = rhs->startColNo;
}

void setAsEndOf(Location* loc, const Location* rhs)
{
    loc->sourceCode = rhs->sourceCode;
    loc->endLineNo = loc->startLineNo = rhs->endLineNo;
    loc->endColNo = loc->startColNo = rhs->endColNo;
}

int compareLocations(const Location* loc1, const Location* loc2)
{
    if ( loc1->sourceCode < loc2->sourceCode )
        return -1;
    if ( loc1->sourceCode > loc2->sourceCode )
        return 1;
    if ( loc1->startLineNo < loc2->startLineNo )
        return -1;
    if ( loc1->startLineNo > loc2->startLineNo )
        return 1;
    if ( loc1->startColNo < loc2->startColNo )
        return -1;
    if ( loc1->startColNo > loc2->startColNo )
        return 1;
    if ( loc1->endLineNo < loc2->endLineNo )
        return -1;
    if ( loc1->endLineNo > loc2->endLineNo )
        return 1;
    if ( loc1->endColNo < loc2->endColNo )
        return -1;
    if ( loc1->endColNo > loc2->endColNo )
        return 1;
    return 0;
}
