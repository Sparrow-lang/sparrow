#pragma once

#include "Nest/Api/EvalMode.h"
#include "Nest/Utils/StringRef.h"

namespace Feather
{
    /// Tests if the given node is a declaration (a node that will expand to a Feather declaration)
    bool isDecl(Node* node);
    /// Tests if the given node is a Feather declaration or has a resuling declaration
    bool isDeclEx(Node* node);
    
    /// Get the name of the given declaration
    /// Throws if the name was not set for the node
    StringRef getName(const Node* decl);

    /// Indicates if the node has associated a name with it
    bool hasName(const Node* decl);

    /// Setter for the name of a declaration
    void setName(Node* decl, StringRef name);
    
    /// Getter for the evaluation mode set in the given declaration (non-ct, ct, auto-ct)
    EvalMode nodeEvalMode(const Node* decl);
    /// Getter for the evaluation mode set in the given declaration (non-ct, ct, auto-ct)
    /// If the given declaration doesn't have an evaluation mode, the evaluation mode of the context is used
    EvalMode effectiveEvalMode(const Node* decl);
    
    /// Setter for the evaluation mode of the given declaration
    void setEvalMode(Node* decl, EvalMode val);

    /// Add the given declaration to the sym tab
    void addToSymTab(Node* decl);

    /// Should we add the given declaration to the symbols table?
    void setShouldAddToSymTab(Node* decl, bool val);
}
