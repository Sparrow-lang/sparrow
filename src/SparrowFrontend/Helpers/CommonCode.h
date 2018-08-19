#pragma once

namespace SprFrontend {
/// Create a constructor call with any number of arguments
Node* createCtorCall(const Location& loc, CompilationContext* context, Nest_NodeRange args);

/// Create a constructor call with one argument
Node* createCtorCall(
        const Location& loc, CompilationContext* context, Node* thisArg, Node* initArg);

/// Create a destructor call
Node* createDtorCall(const Location& loc, CompilationContext* context, Node* thisArg);

/// Creates the code that calls the given function
Node* createFunctionCall(
        const Location& loc, CompilationContext* context, Node* fun, Nest_NodeRange args);

/// Create a temporary variable structure given the construct action for the given variable
Node* createTempVarConstruct(
        const Location& loc, CompilationContext* context, Node* constructAction, Node* var);

/// Assuming the given node points to a function, creates a FunPtr object to refer to that function
Node* createFunPtr(Node* funNode);
} // namespace SprFrontend
