#pragma once

#include "Convert.h"

#include <NodeCommonsH.h>

FWD_CLASS1(Feather, Class);
FWD_CLASS1(SprFrontend, Callable);

namespace SprFrontend
{
    /// Does the function overloading algorithm, selecting what function needs to be called, and returns the code that calls it
    /// The given decls should be a list of Function objects
    DynNode* selectOverload(CompilationContext* context, const Location& loc, Nest::EvalMode evalMode,
        Feather::DynNodeVector decls, Feather::DynNodeVector args,
        bool reportErrors, const string& funName);

    /// Try to search for a conversion constructor of the given class that can take the given argument
    bool selectConversionCtor(CompilationContext* context, Feather::Class* destClass, Nest::EvalMode destMode,
        TypeRef argType, DynNode* arg, DynNode** conv);

    /// Search for a ct-to-rt constructor for the given class; returns the Callable that can be used to perform the call
    Callable* selectCtToRtCtor(CompilationContext* context, TypeRef ctType);
}
