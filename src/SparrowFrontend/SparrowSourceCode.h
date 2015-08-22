#pragma once

/// The kind of the Sparrow source code
extern int SprFe_kindSparrowSourceCode;

/// Register the Sparrow source code
void SprFe_registerSparrowSourceCode();

/// Parses a Sparrow expression
Node* SprFe_parseSparrowExpression(Location loc, const char* code);
