#pragma once

#include "Callable.h"
#include "Nodes/Generics.hpp"

#include "Nest/Utils/cppif/SmallVector.hpp"

namespace SprFrontend {

using Nest::NodeRangeT;
using Nest::SmallVector;
struct ParameterDecl;

/**
 * @brief      Completes the given 'args' with the default values found in the given parameters
 *
 * @param[out] resArgs  The resulting arguments; one argument for each parameter
 * @param[in]  args     The arguments (can be fewer then the number of parameters)
 * @param[in]  params   The parameters
 *
 * @return     False, if we cannot complete the arguments
 *
 * This will make sure to semantic check the init nodes for the parameters used to complete the
 * args.
 *
 * Postconditions (when returns true):
 *     - resArgs.size() == params.size()
 *     - all the initializer used to complete the args are semantically checked
 *
 * Applicable for package and datatype generics, where we don't have dependent params
 */
bool completeArgsWithDefaults(
        SmallVector<NodeHandle>& resArgs, NodeRange args, NodeRangeT<ParameterDecl> params);

/**
 * @brief      Check conversions of args to params by types
 *
 * @param[out] conversions    The conversions to be used for args
 * @param[in]  ccloc          The context & location of the calling code
 * @param[in]  argTypes       The argument types
 * @param[in]  paramTypes     The parameter types
 * @param[in]  customCvtMode  The custom cvt mode
 * @param[in]  reportErrors   True if we should report errors
 *
 * @return     The worse conversion to be applied.
 *
 * This will check if the given arguments can be converted to the given param types. All the checks
 * are done on types only. This function will also return in 'conversions' the conversions that need
 * to be applied to the arguments.
 *
 * Postconditions (if returning a valid conversion):
 * - returned conversion type is the worst of all 'conversions'
 * - there is one conversion for each arg/type
 * - converting the args with the given conversions, will match the given param types
 */
ConversionType checkTypeConversions(SmallVector<ConversionResult>& conversions, CCLoc ccloc,
        Range<Type> argTypes, Range<Type> paramTypes, CustomCvtMode customCvtMode,
        bool reportErrors);

//! Apply the given conversions to the given set of arguments
void applyConversions(SmallVector<NodeHandle>& args, SmallVector<ConversionResult> conversions);

/**
 * @brief      Gets the bound values for the given args; classic algo
 *
 * @param[out] boundVals  The output bound vals
 * @param[in]  args       The arguments (with conversions applied)
 *
 * This is called only for the case of classic CT-only params.
 * It will issue an error if any of the arguments is not CT.
 *
 * Preconditions:
 *     - can compute type for all args
 *     - all args are CT
 *
 * Postconditions:
 *     - boundVals.size() == args.size()
 *     - boundVals[i] == ctEval(args[i])
 */
void getBoundValuesClassic(SmallVector<NodeHandle>& boundVals, NodeRange args);

//! Returns the description to be attached to an instantiated classic generic (datatype / package)
string getGenericDescription(Feather::DeclNode originalDecl, Instantiation inst);

//! Get a human-readable representation of the given generic, with the given params
string genericToStringClassic(Feather::DeclNode decl, NodeRangeT<ParameterDecl> params);

} // namespace SprFrontend
