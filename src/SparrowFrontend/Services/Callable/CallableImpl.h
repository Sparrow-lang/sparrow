#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Services/Callable/Callable.h"
#include "SparrowFrontend/Services/Callable/CallableData.h"

namespace SprFrontend {

struct GenericFunction;
struct GenericDatatype;
struct GenericPackage;
struct ConceptDecl;

struct CallableImpl : Callable {
    CallableData data_;

    CallableImpl(CallableData data)
        : Callable(data.decl)
        , data_(std::move(data)) {}

    /// Checks if we can call this with the given arguments
    /// This method can cache some information needed by the 'generateCall'
    ConversionType canCall(const CCLoc& ccloc, NodeRange args, EvalMode evalMode,
            CustomCvtMode customCvtMode, bool reportErrors = false) override;
    /// Same as above, but makes the check only on type, and not on the actual
    /// argument; doesn't cache any args
    ConversionType canCall(const CCLoc& ccloc, Range<Type> argTypes, EvalMode evalMode,
            CustomCvtMode customCvtMode, bool reportErrors = false) override;

    /// Generates the node that actually calls this callable
    /// This must be called only if 'canCall' method returned a success conversion
    /// type
    NodeHandle generateCall(const CCLoc& ccloc) override;

    /// Gets a string representation of the callable (i.e., function name)
    string toString() const override;

    //! Returns the number of parameters the callable has
    int numParams() const override;

    //! Get the type of the parameter with the given index.
    Type paramType(int idx) const override;
};

Callable* mkFunCallable(Feather::FunctionDecl fun, TypeWithStorage implicitArgType = {});
Callable* mkGenericFunCallable(GenericFunction genericFun, TypeWithStorage implicitArgType = {});
Callable* mkGenericClassCallable(GenericDatatype genericDatatype);
Callable* mkGenericPackageCallable(GenericPackage genericPackage);
Callable* mkConceptCallable(ConceptDecl concept);

} // namespace SprFrontend
