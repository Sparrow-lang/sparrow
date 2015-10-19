#pragma once

#include "Nest/Api/EvalMode.h"

typedef struct Nest_CompilationContext CompilationContext;

namespace Feather
{
    Node* getParentDecl(CompilationContext* context);
    Node* getParentFun(CompilationContext* context);
    Node* getParentClass(CompilationContext* context);
    Node* getParentLoop(CompilationContext* context);

    /// Getter for the context of the node that introduces the sym-tab
    /// It can be the context of the package, class or function
    CompilationContext* getSymTabContext(CompilationContext* context);

    bool isLocal(CompilationContext* context);
    bool isClassMember(CompilationContext* context);
}
