#pragma once

FWD_CLASS1(Feather, Function);

namespace SprFrontend
{
    /// Create a constructor call with any number of arguments
    DynNode* createCtorCall(const Location& loc, CompilationContext* context, DynNodeVector args);

    /// Create a constructor call with one argument
    DynNode* createCtorCall(const Location& loc, CompilationContext* context, DynNode* thisArg, DynNode* initArg);

    /// Create a destructor call
    DynNode* createDtorCall(const Location& loc, CompilationContext* context, DynNode* thisArg);

    /// Creates the code that calls the given function
    DynNode* createFunctionCall(const Location& loc, CompilationContext* context, Feather::Function* fun, DynNodeVector args);

    /// Create a temporary variable structure given the construct action for the given variable
    DynNode* createTempVarConstruct(const Location& loc, CompilationContext* context, DynNode* constructAction, DynNode* var);

    /// Assuming the given node points to a function, creates a FunPtr object to refer to that function
    Node* createFunPtr(Node* funNode);
    DynNode* createFunPtr(DynNode* funNode);
}
