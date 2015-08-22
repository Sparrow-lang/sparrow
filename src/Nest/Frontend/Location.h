#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// A location indicates a region of characters in a particular source code
typedef struct Location_t
{
    const void* sourceCode;     ///< The source code containing this location
    unsigned int startLineNo;   ///< The start line number
    unsigned int startColNo;    ///< The start column number
    unsigned int endLineNo;     ///< The ending line number
    unsigned int endColNo;      ///< The ending column number
} Location;

Location mkEmptyLocation();
Location mkLocation(const void* sourceCode, unsigned int startLineNo, unsigned int startColNo, unsigned int endLineNo, unsigned int endColNo);
Location mkLocation1(const void* sourceCode, unsigned int lineNo, unsigned int colNo);

int isEmpty(const Location* loc);

/// Make the start position to be the same as the end position
void step(Location* loc);

/// Add the given number of columns to the end position; start position remains unchanged
void addColumns(Location* loc, unsigned int count);

/// Add the given number of lines to the end position; start position remains unchanged
void addLines(Location* loc, unsigned int count);

/// Set the start position of this location to the start position of the given location; the end position remains unchanged
void copyStart(Location* loc, const Location* rhs);

/// Set the end position of this location to the end position of the given location; the start position remains unchanged
void copyEnd(Location* loc, const Location* rhs);

/// Set this location with both start and end to equal the start of the given location
void setAsStartOf(Location* loc, const Location* rhs);

/// Set this location with both start and end to equal the end of the given location
void setAsEndOf(Location* loc, const Location* rhs);

/// Compare two locations
int compareLocations(const Location* loc1, const Location* loc2);


#ifdef __cplusplus
}
#endif
