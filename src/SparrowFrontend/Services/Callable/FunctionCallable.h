#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Services/Callable/Callable.h"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Generics.hpp"

#include "Nest/Utils/cppif/SmallVector.hpp"

namespace SprFrontend {

/**
 * @brief      Callable implementation for calling a Feather function
 *
 * This is not a generic. All the parameters need to be RT.
 *
 * The number of args need to match the number of parameters (possible by using default arguments).
 *
 * Constraints:
 *     - all parameters are RT
 *     - does not support concept/dependent parameters
 *
 * @see        Callable
 */
class FunctionCallable : public Callable {
public:
    FunctionCallable(Feather::FunctionDecl fun, TypeWithStorage implicitArgType = {});

    ConversionType canCall(const CCLoc& ccloc, NodeRange args, EvalMode evalMode,
            CustomCvtMode customCvtMode, bool reportErrors = false) override;
    ConversionType canCall(const CCLoc& ccloc, Range<Type> argTypes, EvalMode evalMode,
            CustomCvtMode customCvtMode, bool reportErrors = false) override;
    NodeHandle generateCall(const CCLoc& ccloc) override;
    string toString() const override;
    int numParams() const override;
    Type paramType(int idx) const override;

private:
    //! This is set for a class-ctor callable to add an implicit this argument when calling the
    //! underlying callable. This is the type of the argument to be added.
    Nest::TypeWithStorage implicitArgType_;
    //! The parameters of the function
    //! Typically ParameterDecl, but can also be Feather::VarDecl
    Nest::NodeRangeT<Feather::DeclNode> params_;
    //! True if the function has that 'autoCt' modifier
    const bool autoCt_{false};
    //! The variable created for implicit this (class-ctor case)
    Feather::VarDecl tmpVar_;

    //! The conversion of the argument
    ConversionResult conv_;
    //! The arguments to be used for the callable
    //! Filled by 'canCall'
    Nest::SmallVector<NodeHandle> args_;
    //! The conversions needed for the arguments
    //! Filled by 'canCall'
    Nest::SmallVector<ConversionResult> conversions_;
};

} // namespace SprFrontend
