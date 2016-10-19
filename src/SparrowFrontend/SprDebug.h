#pragma once

#include "Nest/Api/NodeRange.h"

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
bool atLocation(Location loc, const char* filename, int startLine = 0, int startCol = 0, int endLine = 0, int endCol = 0);


/**
 * Print a start of a scope
 *
 * This will print the job name and the 'url' as parameter and then starts a scope
 * The following debug prints will be indented accordingly.
 * The scope will be ended with printEnd()
 *
 * @param job          Name of the job to start (optional, can be NULL)
 * @param url          Some information about what job we are printing
 * @param continueLine If true, don't assume we are the start of the line, and
 *                     don't add any spaces
 */
void printStart(const char* job, const char* url, bool continueLine = false);
/// Closes a scope started by printStart()
void printEnd();

/// Print the given expression node
void printExpNode(Node* node, bool printUnknown = true);

/// Print the given node structure (assuming top-level node is given)
void printNodeStructure(Node* node, bool continueLine = false);
