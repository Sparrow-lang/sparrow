#pragma once

#include <Nest/Intermediate/EvalMode.h>

FWD_CLASS1(Nest, DynNode);

namespace Feather
{
    using Nest::DynNode;
    using Nest::EvalMode;

    /// Tests if the given node is a declaration (a node that will expand to a Feather declaration)
    bool isDecl(DynNode* node);
    /// Tests if the given node is a Feather declaration or has a resuling declaration
    bool isDeclEx(DynNode* node);
    
    /// Get the name of the given declaration
    /// Throws if the name was not set for the node
    const string& getName(const DynNode* decl);

    /// Indicates if the node has associated a name with it
    bool hasName(const DynNode* decl);

    /// Setter for the name of a declaration
    void setName(DynNode* decl, string name);
    
    /// Getter for the evaluation mode set in the given declaration (non-ct, ct, auto-ct)
    EvalMode nodeEvalMode(const DynNode* decl);
    /// Getter for the evaluation mode set in the given declaration (non-ct, ct, auto-ct)
    /// If the given declaration doesn't have an evaluation mode, the evaluation mode of the context is used
    EvalMode effectiveEvalMode(const DynNode* decl);
    
    /// Setter for the evaluation mode of the given declaration
    void setEvalMode(DynNode* decl, EvalMode val);

    /// Add the given declaration to the sym tab
    void addToSymTab(DynNode* decl);

    /// Should we add the given declaration to the symbols table?
    void setShouldAddToSymTab(DynNode* decl, bool val);
}
