#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Services/Callable/Callable.h"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Generics.hpp"

#include "Nest/Utils/cppif/SmallVector.hpp"

namespace SprFrontend {

struct GenericDatatype;

/**
 * @brief      Callable implementation for calling a generic datatype
 *
 * This represents a classic generic call. All the parameters of the datatypes need to be CT, and
 * therefore all the args need to be CT.
 *
 * The number of args need to match the number of parameters (possible by using default arguments).
 *
 * Generic datatype callables cannot be called by using just the arg types.
 *
 * There are several reasons for which we cannot call a generic datatype:
 *     - number of arguments mismatch
 *     - argument types don't match the parameter types
 *     - cannot instantiate the generic (if clause)
 * In all other cases, we should be able to instantiate generic datatype callables.
 *
 * A call to a generic datatype would yield a DeclExp pointing to the actual instantiated datatype.
 *
 * Constraints:
 *     - does not support concept parameters
 *     - does not support dependent parameters
 *
 * @see        Callable
 */
class GenericDatatypeCallable : public Callable {
public:
    GenericDatatypeCallable(GenericDatatype decl);

    ConversionType canCall(const CCLoc& ccloc, NodeRange args, EvalMode evalMode,
            CustomCvtMode customCvtMode, bool reportErrors = false) override;
    ConversionType canCall(const CCLoc& ccloc, Range<Type> argTypes, EvalMode evalMode,
            CustomCvtMode customCvtMode, bool reportErrors = false) override;
    NodeHandle generateCall(const CCLoc& ccloc) override;
    string toString() const override;
    int numParams() const override;
    Type paramType(int idx) const override;

private:
    //! The parameters of the generic callable
    Nest::NodeRangeT<ParameterDecl> params_;
    //! The arguments to be used for the callable
    //! Filled by 'canCall'
    Nest::SmallVector<NodeHandle> args_;
    //! The generic instantiation computed at the end of 'canCall'
    Instantiation genericInst_;

    //! Create the instantiated datatype by cloning the original decl.
    DataTypeDecl createInstantiatedDatatype();
};

} // namespace SprFrontend
