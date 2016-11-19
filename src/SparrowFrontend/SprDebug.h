#pragma once

#include "Nest/Api/Node.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Used to easily test if we are at a given location
 *
 * We test if the given location has the given attributes.
 *
 * If filename is non-null, we test that the source-code url ends with the given
 * string.
 *
 * If the rest of the parameters are non-zero, we test the location matches those
 * parameter.
 *
 * Any zero (or NULL) parameter is ignored, and location is not tested for that
 * parameter.
 *
 * @return True if the given location matches the parameters
 */
int atLocation(Location loc, const char* filename, int startLine, int startCol, int endLine, int endCol);


/// Starts a scope, with an open curly
void printStart();
/// Closes a scope started by printStart()
void printEnd();
/// Print the spaces corresponding to the current indent level
void printSpaces();

/// Print the given node to the console, in a pretty fashion
void printNode(const Node* node);


#ifdef __cplusplus
}
#endif
