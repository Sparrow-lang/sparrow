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

    ConversionType canCall(const CCLoc& ccloc, NodeRange args, EvalMode evalMode,
            CustomCvtMode customCvtMode, bool reportErrors = false) override;
    ConversionType canCall(const CCLoc& ccloc, Range<Type> argTypes, EvalMode evalMode,
            CustomCvtMode customCvtMode, bool reportErrors = false) override;
    NodeHandle generateCall(const CCLoc& ccloc) override;
    string toString() const override;
    int numParams() const override;
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
