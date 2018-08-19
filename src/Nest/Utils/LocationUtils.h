#pragma once

#include "Nest/Api/Location.h"

#ifdef __cplusplus
extern "C" {
#endif

Nest_Location Nest_mkEmptyLocation();
Nest_Location Nest_mkLocation(const Nest_SourceCode* sourceCode, unsigned int startLineNo,
        unsigned int startColNo, unsigned int endLineNo, unsigned int endColNo);
Nest_Location Nest_mkLocation1(
        const Nest_SourceCode* sourceCode, unsigned int lineNo, unsigned int colNo);

/// Check if the given location is empty; true if it has no source code
int Nest_isLocEmpty(const Nest_Location* loc);

/// Compare two locations
int Nest_compareLocations(const Nest_Location* loc1, const Nest_Location* loc2);

#ifdef __cplusplus
}
#endif
