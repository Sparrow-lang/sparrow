#include <StdInc.h>
#include "SparrowFrontend/Services/Callable/GenericDatatypeCallable.h"
#include "SparrowFrontend/Services/Callable/CallableHelpers.h"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Nodes/Exp.hpp"

#include "Nest/Utils/cppif/SmallVector.hpp"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace SprFrontend {

namespace {
/**
 * Get the resulting eval mode for a generic datatype instantiation.
 *
 * This will be used for the instantiation of the generic datatype.
 *
 * This checks all the bound values that are types. If all the bound types are CT types, then we
 * return a CT-only mode. If all the bound types are RT-only, then we return a RT-only mode.
 * For the rest of the cases (mixed modes, no types, etc.) We return the mainEvalMode value.
 *
 * @param mainEvalMode The effective eval mode of the generic decl
 * @param boundValues  The bound values to be used for instantiating the generic
 *
 * @return The eval mode that should be used for the instantiation.
 */
EvalMode getResultingEvalMode(EvalMode mainEvalMode, NodeRange boundValues) {
    bool hasCtOnlyArgs = false;
    for (NodeHandle boundVal : boundValues) {
        if (!boundVal)
            continue;
        ASSERT(!boundVal || boundVal.type());
        // Test the type given to the 'Type' parameters (i.e., we need to know
        // if Vector(t) can be rtct based on the mode of t)
        Type t = tryGetTypeValue(boundVal);
        if (t) {
            if (t.mode() == modeCt)
                hasCtOnlyArgs = true;
        } else if (!boundVal.type().canBeUsedAtRt()) {
            hasCtOnlyArgs = true;
        }
    }

    if (hasCtOnlyArgs)
        return modeCt;
    return mainEvalMode;
}

} // namespace

GenericDatatypeCallable::GenericDatatypeCallable(GenericDatatype decl)
    : Callable(decl)
    , params_(decl.instSet().params()) {}

ConversionType GenericDatatypeCallable::canCall(const CCLoc& ccloc, NodeRange args,
        EvalMode evalMode, CustomCvtMode customCvtMode, bool reportErrors) {

    // This can be called a second time to report the errors
    genericInst_ = Instantiation();
    args_.clear();

    // Complete the missing args with defaults
    if (!completeArgsWithDefaults(args_, args, params_))
        return convNone;

    // Check argument count
    int paramsCount = params_.size();
    if (paramsCount != args_.size()) {
        if (reportErrors) {
            REP_INFO(NOLOC, "Different number of parameters; args=%1%, params=%2%") % args_.size() %
                    paramsCount;
        }
        return convNone;
    }

    // Get the arg types to perform the check on types
    SmallVector<Type> argTypes;
    argTypes.reserve(args_.size());
    for (auto arg : args_)
        argTypes.push_back(arg.type());

    // Get the param types
    SmallVector<Type> paramTypes;
    paramTypes.reserve(params_.size());
    for (auto p : params_) {
        if (!p || !p.type())
            return convNone;
        paramTypes.push_back(p.type());
    }

    // Do the checks on types
    // Make sure that all argument types are convertible to param types
    // Capture the conversions required.
    SmallVector<ConversionResult> conversions;
    ConversionType res = checkTypeConversions(
            conversions, ccloc, argTypes, paramTypes, customCvtMode, reportErrors);
    if (!res)
        return convNone;

    // At this point, we know that if there are no if clauses, we can call the generic

    // Apply the conversions to the arguments
    applyConversions(args_, conversions);

    // Check if we can instantiate the generic with the given arguments
    // (with conversions applied)
    // Note: we overwrite the args with their conversions;
    // We don't use the old arguments anymore
    // args_ = argsWithConversion(data_);
    GenericDatatype genNode = GenericDatatype(decl_);

    // Get the bound values to be used to instantiate the generic datatype
    SmallVector<NodeHandle> boundVals;
    getBoundValuesClassic(boundVals, NodeRangeT<NodeHandle>{args_.all()});

    EvalMode resultingEvalMode =
            getResultingEvalMode(decl_.effectiveMode(), NodeRangeT<NodeHandle>(boundVals));

    genericInst_ =
            canInstantiate(genNode.instSet(), NodeRangeT<NodeHandle>(boundVals), resultingEvalMode);
    if (!genericInst_ && reportErrors) {
        REP_INFO(NOLOC, "Cannot instantiate generic datatype");
    }
    if (!genericInst_)
        return convNone;

    return res;
}
ConversionType GenericDatatypeCallable::canCall(const CCLoc& ccloc, Range<Type> argTypes,
        EvalMode evalMode, CustomCvtMode customCvtMode, bool reportErrors) {
    // Cannot call only with types
    return convNone;
}

NodeHandle GenericDatatypeCallable::generateCall(const CCLoc& ccloc) {

    ASSERT(genericInst_);

    // If not already created, create the actual instantiation declaration
    NodeHandle instDecl = genericInst_.instantiatedDecl();
    if (!instDecl) {
        instDecl = createInstantiatedDatatype();

        if (!instDecl.computeType())
            return {};
        Nest_queueSemanticCheck(instDecl);
        genericInst_.setInstantiatedDecl(instDecl);

        // Add the instantiated datatype as an additional node to the generic node
        decl_.addAdditionalNode(genericInst_.boundVarsNode());
        if (decl_.curExplanation())
            decl_.curExplanation().addAdditionalNode(genericInst_.boundVarsNode());
    }

    // The result is a type node (CT), pointing to the instantiated datatype
    auto structDecl = instDecl.explanation().kindCast<Feather::StructDecl>();
    ASSERT(structDecl);
    return createTypeNode(
            decl_.context(), ccloc.loc_, Feather::DataType::get(structDecl, 0, modeRt));
}

int GenericDatatypeCallable::numParams() const { return params_.size(); }

Type GenericDatatypeCallable::paramType(int idx) const {
    ASSERT(idx < params_.size());
    auto param = params_[idx];
    ASSERT(param);
    return param.type();
}

string GenericDatatypeCallable::toString() const { return genericToStringClassic(decl_, params_); }

DataTypeDecl GenericDatatypeCallable::createInstantiatedDatatype() {
    DataTypeDecl orig = GenericDatatype(decl_).original();

    // Place the instantiated datatype in the context where we can access the bound variables
    CompilationContext* ctx = genericInst_.boundVarsNode().childrenContext();

    auto body = orig.body();
    body = body ? body.clone() : NodeList();
    auto res = DataTypeDecl::create(orig.location(), orig.name(), nullptr, nullptr, nullptr, body);
    copyAccessType(res, orig);
    copyModifiersSetMode(orig, res, ctx->evalMode);
    res.setContext(ctx);

    // Set the proper description of the generic
    res.setProperty(propDescription, getGenericDescription(orig, genericInst_));

    return res;
}

} // namespace SprFrontend
