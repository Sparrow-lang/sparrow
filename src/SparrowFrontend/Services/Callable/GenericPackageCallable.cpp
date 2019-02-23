#include <StdInc.h>
#include "SparrowFrontend/Services/Callable/GenericPackageCallable.h"
#include "SparrowFrontend/Services/Callable/CallableHelpers.h"
#include "SparrowFrontend/Services/IConceptsService.h"
#include "SparrowFrontend/Services/IConvertService.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Nodes/Exp.hpp"

#include "Nest/Utils/cppif/SmallVector.hpp"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace SprFrontend {

GenericPackageCallable::GenericPackageCallable(GenericPackage decl)
    : Callable(decl)
    , params_(decl.instSet().params()) {}

ConversionType GenericPackageCallable::canCall(const CCLoc& ccloc, NodeRange args,
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
    GenericPackage genNode = GenericPackage(decl_);

    // Get the bound values to be used to instantiate the generic package
    SmallVector<NodeHandle> boundVals;
    getBoundValuesClassic(boundVals, args);

    genericInst_ =
            canInstantiate(genNode.instSet(), NodeRangeT<NodeHandle>(boundVals), modeUnspecified);
    if (!genericInst_ && reportErrors) {
        REP_INFO(NOLOC, "Cannot instantiate generic package");
    }
    if (!genericInst_)
        return convNone;

    return res;
}
ConversionType GenericPackageCallable::canCall(const CCLoc& ccloc, Range<Type> argTypes,
        EvalMode evalMode, CustomCvtMode customCvtMode, bool reportErrors) {
    // Cannot call only with types
    return convNone;
}

NodeHandle GenericPackageCallable::generateCall(const CCLoc& ccloc) {

    ASSERT(genericInst_);

    // If not already created, create the actual instantiation declaration
    NodeHandle instDecl = genericInst_.instantiatedDecl();
    if (!instDecl) {
        instDecl = createInstantiatedPackage();

        if (!instDecl.computeType())
            return {};
        Nest_queueSemanticCheck(instDecl);
        genericInst_.setInstantiatedDecl(instDecl);

        // Add the instantiated package as an additional node to the generic node
        decl_.addAdditionalNode(genericInst_.boundVarsNode());
        if (decl_.curExplanation())
            decl_.curExplanation().addAdditionalNode(genericInst_.boundVarsNode());
    }

    // The result is a declaration expression pointing to the instantiated package
    auto res = DeclExp::create(ccloc.loc_, NodeRange({instDecl}), nullptr);
    res.setContext(ccloc.context_);

    return res;
}

int GenericPackageCallable::numParams() const { return params_.size(); }

Type GenericPackageCallable::paramType(int idx) const {
    ASSERT(idx < params_.size());
    auto param = params_[idx];
    ASSERT(param);
    return param.type();
}

string GenericPackageCallable::toString() const { return genericToStringClassic(decl_, params_); }

PackageDecl GenericPackageCallable::createInstantiatedPackage() {
    PackageDecl orig = GenericPackage(decl_).original();

    // Place the instantiated package in the context where we can access the bound variables
    CompilationContext* ctx = genericInst_.boundVarsNode().childrenContext();

    auto body = orig.body();
    body = body ? body.clone() : NodeList();
    auto res = PackageDecl::create(orig.location(), orig.name(), body);
    copyAccessType(res, orig);
    copyModifiersSetMode(orig, res, ctx->evalMode);
    res.setContext(ctx);

    // Set the proper description of the generic
    genericInst_.setProperty(propDescription, getGenericDescription(orig, genericInst_));

    return res;
}

} // namespace SprFrontend
