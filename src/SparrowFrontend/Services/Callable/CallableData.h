#pragma once

#include "SparrowFrontend/NodeCommonsH.h"

namespace SprFrontend {

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

    /// The decls we want to call
    Feather::DeclNode decl;
    /// The parameters of the decl to call
    /// For Feather functions these are variables, for others, these are ParameterDecl
    NodeRange params;
    /// True if we need to call the function in autoCt mode
    bool autoCt;

    /// The arguments used to call the callable
    /// If the callable has default parameters, this will be extended
    /// Computed by 'canCall'
    vector<NodeHandle> args;

    /// The conversions needed for each argument
    /// Computed by 'canCall'
    vector<ConversionResult> conversions;

    /// This is set for class-ctor callables to add an implicit this argument
    /// when calling the underlying callable. This is the type of the argument
    /// to be added.
    Nest::TypeWithStorage implicitArgType;

    /// Temporary data: the generic instantiation (generic case)
    Node* genericInst;
    /// Temporary data: the variable created for implicit this (class-ctor case)
    Node* tmpVar;

    CallableData()
        : type(CallableType::function)
        , decl{nullptr}
        , params{nullptr, nullptr}
        , autoCt{false}
        , implicitArgType(nullptr)
        , genericInst(nullptr)
        , tmpVar(nullptr) {}
};

} // namespace SprFrontend
