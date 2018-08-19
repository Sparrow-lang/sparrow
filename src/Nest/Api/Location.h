#pragma once

typedef struct Nest_SourceCode Nest_SourceCode;

struct Nest_LocationPos {
    unsigned int line; ///< Line number (starting with 1)
    unsigned int col;  ///< Column number (starting with 1)
};
typedef struct Nest_LocationPos Nest_LocationPos;

/// A location indicates a region of characters in a particular source code
struct Nest_Location {
    const Nest_SourceCode* sourceCode; ///< The source code containing this location
    Nest_LocationPos start;            ///< The start position in the source code
    Nest_LocationPos end;              ///< The end position in the source code
};

typedef struct Nest_Location Nest_Location;
