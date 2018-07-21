#include "Nest/Utils/LocationUtils.h"
#include "Nest/Utils/Assert.h"

Nest_Location Nest_mkEmptyLocation() {
    Nest_Location loc = {0, {1, 1}, {1, 1}};
    return loc;
}
Nest_Location Nest_mkLocation(const Nest_SourceCode* sourceCode, unsigned int startLineNo,
        unsigned int startColNo, unsigned int endLineNo, unsigned int endColNo) {
    Nest_Location loc = {sourceCode, {startLineNo, startColNo}, {endLineNo, endColNo}};
    return loc;
}
Nest_Location Nest_mkLocation1(const Nest_SourceCode* sourceCode, unsigned int lineNo, unsigned int colNo) {
    Nest_Location loc = {sourceCode, {lineNo, colNo}, {lineNo, colNo}};
    return loc;
}

int Nest_isLocEmpty(const Nest_Location* loc) { return loc->sourceCode == 0; }

int Nest_compareLocations(const Nest_Location* loc1, const Nest_Location* loc2) {
    if (loc1->sourceCode < loc2->sourceCode)
        return -1;
    if (loc1->sourceCode > loc2->sourceCode)
        return 1;
    if (loc1->start.line < loc2->start.line)
        return -1;
    if (loc1->start.line > loc2->start.line)
        return 1;
    if (loc1->start.col < loc2->start.col)
        return -1;
    if (loc1->start.col > loc2->start.col)
        return 1;
    if (loc1->end.line < loc2->end.line)
        return -1;
    if (loc1->end.line > loc2->end.line)
        return 1;
    if (loc1->end.col < loc2->end.col)
        return -1;
    if (loc1->end.col > loc2->end.col)
        return 1;
    return 0;
}
