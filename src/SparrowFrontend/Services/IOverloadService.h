#pragma once

#include "SparrowFrontend/NodeCommonsH.h"

namespace SprFrontend {

/// Describes the error reporting to be used when overloading fails.
enum class OverloadReporting {
    none,       //!< No errors should be given
    noTopLevel, //!< No top level error, but just the info about the candidates
    full,       //!< Full error reporting, including a top-level error
};

//! The interface for the service that deals with overload selection.
//! Used so that we can easily mock and replace this service.
struct IOverloadService {
    virtual ~IOverloadService() {}

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
    virtual Node* selectOverload(CompilationContext* context, const Location& loc,
            EvalMode evalMode, Nest_NodeRange decls, Nest_NodeRange args,
            OverloadReporting errReporting, StringRef funName) = 0;

    /// Try to search for a conversion constructor of the given class that can take
    /// the given argument
    virtual bool selectConversionCtor(
            CompilationContext* context, Node* destClass, EvalMode destMode, Type argType) = 0;

    /// Search for a ct-to-rt constructor for the given argument
    /// Returns the node to be used to call the ctor; null on failure
    virtual Node* selectCtToRtCtor(Node* ctArg) = 0;
};

//! The overload service instance that we are using across the Sparrow compiler
extern unique_ptr<IOverloadService> g_OverloadService;

} // namespace SprFrontend
