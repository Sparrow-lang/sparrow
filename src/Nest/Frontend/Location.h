#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_SourceCode SourceCode;

struct Nest_LocationPos {
    unsigned int line;  ///< Line number (starting with 1)
    unsigned int col;   ///< Column number (starting with 1)
};
typedef struct Nest_LocationPos Nest_LocationPos;
typedef struct Nest_LocationPos LocationPos;

/// A location indicates a region of characters in a particular source code
struct Nest_Location {
    const SourceCode* sourceCode;   ///< The source code containing this location
    Nest_LocationPos start;         ///< The start position in the source code
    Nest_LocationPos end;           ///< The end position in the source code
};

typedef struct Nest_Location Nest_Location;
typedef struct Nest_Location Location;

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
