#pragma once

#include <Nest/Intermediate/EvalMode.h>

FWD_CLASS1(Nest, CompilationContext);
FWD_CLASS1(Feather, DynNode);
FWD_CLASS1(Feather, Function);
FWD_CLASS1(Feather, Class);

namespace Feather
{
    using Nest::CompilationContext;
    using Nest::Node;
    
    Node* getParentDecl(CompilationContext* context);
    Function* getParentFun(CompilationContext* context);
    Class* getParentClass(CompilationContext* context);
    Node* getParentLoop(CompilationContext* context);

    /// Getter for the context of the node that introduces the sym-tab
    /// It can be the context of the package, class or function
    CompilationContext* getSymTabContext(CompilationContext* context);

    bool isLocal(CompilationContext* context);
    bool isClassMember(CompilationContext* context);
}
