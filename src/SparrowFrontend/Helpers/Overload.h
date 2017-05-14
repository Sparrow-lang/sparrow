#pragma once

#include "Convert.h"

#include <NodeCommonsH.h>

FWD_CLASS1(SprFrontend, Callable);

namespace SprFrontend {

/// Describes the error reporting to be used when overloading fails.
enum class OverloadReporting {
    none,       //!< No errors should be given
    noTopLevel, //!< No top level error, but just the info about the candidates
    full,       //!< Full error reporting, including a top-level error
};

/**
 * Applies the function overloading flow.
 *
 * Selects the function needs to be called and returns the code that calls
 * it. Returns null if overloading failed.
 *
 * The given decls should be a list of declarations that are callable.
 *
 * @param context      The context from which we want to start overload
 * @param loc          The location from which we start the overload process
 * @param evalMode     The evaluation mode to be used when calling
 * @param decls        The list of "callable" declarations
 * @param args         The arguments passed to the function to be called
 * @param errReporting How we should we report failures
 * @param funName      The name of the function to be called (used for
 * reporting)
 *
 * @return The node used to called the selected overload; null on failure
 */
Node* selectOverload(CompilationContext* context, const Location& loc,
                     EvalMode evalMode, NodeRange decls, NodeRange args,
                     OverloadReporting errReporting, StringRef funName);

/// Try to search for a conversion constructor of the given class that can take
/// the given argument
bool selectConversionCtor(CompilationContext* context, Node* destClass,
                          EvalMode destMode, TypeRef argType, Node* arg,
                          Node** conv);

/// Search for a ct-to-rt constructor for the given argument
/// Returns the node to be used to call the ctor; null on failure
Node* selectCtToRtCtor(Node* ctArg);
}
