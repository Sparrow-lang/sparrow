#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Services/Callable/Callable.h"
#include "SparrowFrontend/Services/Callable/CallableData.h"
#include "SparrowFrontend/Nodes/Decl.hpp"

namespace SprFrontend {

struct ConceptDecl;

/**
 * @brief      Callable implementation for calling concepts
 *
 * This is used for fun applications like 'Concept(exp)' to determine if the type of the expression
 * fulfills the concept.
 *
 * The 'canCall' always returns a valid conversion, as we can always have a valid expression. But,
 * depending on the type and on the concept, the fun application may be expanded into 'true' or
 * 'false' compile-time constants.
 *
 * The callable always has one parameter. This parameter has the unconstrained concept type.
 *
 * @see        Callable
 */
class ConceptCallable : public Callable {
public:
    ConceptCallable(ConceptDecl concept);

    /// Checks if we can call this with the given arguments
    /// This method can cache some information needed by the 'generateCall'
    ConversionType canCall(const CCLoc& ccloc, NodeRange args, EvalMode evalMode,
            CustomCvtMode customCvtMode, bool reportErrors = false) override;
    /// Same as above, but makes the check only on type, and not on the actual
    /// argument; doesn't cache any args
    ConversionType canCall(const CCLoc& ccloc, const vector<Type>& argTypes, EvalMode evalMode,
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

private:
    //! The expected type (ConceptType)
    TypeWithStorage paramType_;
    //! The argument passed to 'canCall'
    NodeHandle argument_;
    //! The conversion of the argument
    ConversionResult conv_;
};

} // namespace SprFrontend
