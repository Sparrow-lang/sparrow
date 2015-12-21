#pragma once

#include "Nest/Api/Location.h"

#ifdef __cplusplus
extern "C" {
#endif

Location Nest_mkEmptyLocation();
Location Nest_mkLocation(const SourceCode* sourceCode, unsigned int startLineNo, unsigned int startColNo, unsigned int endLineNo, unsigned int endColNo);
Location Nest_mkLocation1(const SourceCode* sourceCode, unsigned int lineNo, unsigned int colNo);

/// Check if the given location is empty; true if it has no source code
int Nest_isLocEmpty(const Location* loc);

/// Compare two locations
int Nest_compareLocations(const Location* loc1, const Location* loc2);


#ifdef __cplusplus
}
#endif
