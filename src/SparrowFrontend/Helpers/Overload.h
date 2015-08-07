#pragma once

#include "Convert.h"

#include <NodeCommonsH.h>

FWD_CLASS1(SprFrontend, Callable);

namespace SprFrontend
{
    /// Does the function overloading algorithm, selecting what function needs to be called, and returns the code that calls it
    /// The given decls should be a list of Function objects
    Node* selectOverload(CompilationContext* context, const Location& loc, Nest::EvalMode evalMode,
        Feather::NodeVector decls, Feather::NodeVector args,
        bool reportErrors, const string& funName);

    /// Try to search for a conversion constructor of the given class that can take the given argument
    bool selectConversionCtor(CompilationContext* context, Node* destClass, Nest::EvalMode destMode,
        TypeRef argType, Node* arg, Node** conv);

    /// Search for a ct-to-rt constructor for the given class; returns the Callable that can be used to perform the call
    Callable* selectCtToRtCtor(CompilationContext* context, TypeRef ctType);
}
