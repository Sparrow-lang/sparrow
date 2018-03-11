#pragma once

#include <NodeCommonsH.h>

#include <Helpers/Convert.h>

#include "Nest/Utils/NodeVector.hpp"

#include <boost/function.hpp>

namespace SprFrontend {

/// Indicates whether we should apply custom conversions for the params
enum CustomCvtMode {
    noCustomCvt,         ///< No custom conversions are allowed
    noCustomCvtForFirst, ///< No custom conversions for the first param
    allowCustomCvt,      ///< Allow custom conversions for all params
};

/// The type of a callable entity
enum class CallableType {
    function,
    genericFun,
    genericClass,
    genericPackage,
    concept,
};

/// Data build to evaluate a callable.
/// Holds additional data about the callable, and data used in the process
/// of determining whether the decl is callable, and generating the actual call
struct CallableData {
    /// The type of callable that we have
    CallableType type;

    /// Indicates if the callable is valid (it hasn't been excluded)
    bool valid;

    /// The decls we want to call
    Node* decl;
    /// The parameters of the decl to call
    NodeRange params;
    /// True if we need to call the function in autoCt mode
    bool autoCt;

    /// The arguments used to call the callable
    /// If the callable has default parameters, this will be extended
    /// Computed by 'canCall'
    NodeVector args;

    /// The conversions needed for each argument
    /// Computed by 'canCall'
    vector<ConversionResult> conversions;

    /// This is set for class-ctor callables to add an implicit this argument
    /// when calling the underlying callable. This is the type of the argument
    /// to be added.
    TypeRef implicitArgType;

    /// Temporary data: the generic instantiation (generic case)
    Node* genericInst;
    /// Temporary data: the variable created for implicit this (class-ctor case)
    Node* tmpVar;

    CallableData()
        : type(CallableType::function), valid(true), decl{nullptr}, params{nullptr, nullptr},
          autoCt{false}, implicitArgType(nullptr), genericInst(nullptr), tmpVar(nullptr) {}
};

//! A vector of callables
typedef vector<CallableData> Callables;

/*

The process of using callables is the following:

- we gather all the callables
- we use 'canCall' to check which callable is compatible with the given
arguments
- we call moreSpecialized to check which callable is more specialized
- we finally call generateCall to generate the code for the actual call

After 'canCall' is called, we know the actual types to be used for generating
the call.

 */

/// Given a declaration, try to gets a list of Callable objects from it.
/// Returns an empty list if the declaration is not callable
void getCallables(NodeRange decls, EvalMode evalMode, Callables& res);

/// Same as above, but apply a filter to all the callables that we have
/// We keep all the decls for which the predicate returns true
void getCallables(NodeRange decls, EvalMode evalMode, Callables& res,
                  const boost::function<bool(Node*)>& pred,
                  const char* ctorName = "ctor");

/// Checks if we can call this with the given arguments
/// This method can cache some information needed by the 'generateCall'
ConversionType canCall(CallableData& c, CompilationContext* context,
                       const Location& loc, NodeRange args, EvalMode evalMode,
                       CustomCvtMode customCvtMode, bool reportErrors = false);
/// Same as above, but makes the check only on type, and not on the actual
/// argument; doesn't cache any args
ConversionType canCall(CallableData& c, CompilationContext* context,
                       const Location& loc, const vector<TypeRef>& argTypes,
                       EvalMode evalMode, CustomCvtMode customCvtMode,
                       bool reportErrors = false);

/// Returns who of two candidates is more specialized.
/// Returns:
///     -1 if f1 is more specialized than f2,
///     1 if f2 is more specialized than f1;
///     0 if neither is more specialized
int moreSpecialized(CompilationContext* context, const CallableData& f1,
                    const CallableData& f2, bool noCustomCvt = false);

/// Generates the node that actually calls this callable
/// This must be called only if 'canCall' method returned a success conversion
/// type
Node* generateCall(CallableData& c, CompilationContext* context,
                   const Location& loc);

/// Getter for the location of this callable (i.e., location of the function)
const Location& location(const CallableData& c);

/// Gets a string representation of the callable (i.e., function name)
string toString(const CallableData& c);
}
