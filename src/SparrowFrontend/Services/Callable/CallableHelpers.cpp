#include <StdInc.h>
#include "SparrowFrontend/Services/Callable/CallableHelpers.h"
#include "SparrowFrontend/Services/IConvertService.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "Nodes/Decl.hpp"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace SprFrontend {

bool completeArgsWithDefaults(
        SmallVector<NodeHandle>& resArgs, NodeRange args, NodeRangeT<ParameterDecl> params) {
    // Copy the list of arguments; add default values if arguments are missing
    int paramsCount = params.size();
    resArgs.clear();
    resArgs.reserve(paramsCount);
    resArgs.insert(args);
    for (int i = args.size(); i < paramsCount; ++i) {
        auto param = params[i];
        if (!param)
            return false;
        auto defaultArg = param.init();
        if (!defaultArg)
            return false; // We have a non-default parameter but we don't have an argument for that
        if (!defaultArg.semanticCheck()) // Make sure this is semantically checked
            return false;

        resArgs.push_back(defaultArg);
    }
    return true;
}

ConversionType checkTypeConversions(SmallVector<ConversionResult>& conversions, CCLoc ccloc,
        Range<Type> argTypes, Range<Type> paramTypes, CustomCvtMode customCvtMode,
        bool reportErrors) {
    int paramsCount = paramTypes.size();
    ASSERT(argTypes.size() == paramsCount);
    conversions.resize(paramsCount, convNone);

    ConversionType res = convDirect;
    for (int i = 0; i < paramsCount; ++i) {
        Type argType = argTypes[i];
        ASSERT(argType);
        Type paramType = paramTypes[i];
        ASSERT(paramType);

        ConversionFlags flags = flagsDefault;
        if (customCvtMode == noCustomCvt || (customCvtMode == noCustomCvtForFirst && i == 0))
            flags = flagDontCallConversionCtor;
        conversions[i] =
                g_ConvertService->checkConversion(ccloc.context_, argType, paramType, flags);
        if (!conversions[i]) {
            if (reportErrors)
                REP_INFO(NOLOC, "Cannot convert argument %1% from %2% to %3%") % (i + 1) % argType %
                        paramType;
            return convNone;
        }

        // Retain the minimum conversion
        if (conversions[i].conversionType() < res)
            res = conversions[i].conversionType();
    }
    return res;
}

void applyConversions(SmallVector<NodeHandle>& args, SmallVector<ConversionResult> conversions) {
    ASSERT(args.size() == conversions.size());
    for (int i = 0; i < args.size(); ++i)
        args[i] = conversions[i].apply(args[i]);
}

void getBoundValuesClassic(SmallVector<NodeHandle>& boundVals, NodeRange args) {
    boundVals.clear();
    boundVals.reserve(args.size());

    for (int i = 0; i < args.size(); ++i) {
        auto arg = args[i];

        // Evaluate the node and add the resulting CtValue as a bound argument
        if (!arg.computeType()) {
            REP_INTERNAL(arg.location(), "Cannot compute type of argument %1%") % arg;
            return;
        }
        if (!Feather_isCt(arg))
            REP_INTERNAL(arg.location(), "Argument must be CT; current type: %1% (node: %2%)") %
                    arg.type() % Nest_toStringEx(arg);
        NodeHandle n = Nest_ctEval(arg);
        // if (!n || n.kind() != nkFeatherExpCtValue)
        //     REP_INTERNAL(arg.location(), "Invalid argument %1% when instantiating generic") %
        //             (i + 1);
        ASSERT(n && n.type());
        boundVals.push_back(n);
    }
}

string getGenericDescription(Feather::DeclNode originalDecl, Instantiation inst) {
    ostringstream oss;
    oss << originalDecl.name() << '[';
    auto boundValues = inst.boundValues();
    bool first = true;
    for (auto val : boundValues) {
        if (!first)
            oss << ", ";
        first = false;
        Type t = evalTypeIfPossible(val);
        if (t)
            oss << t;
        else
            oss << val;
    }
    oss << ']';
    return oss.str();
}

string genericToStringClassic(DeclNode decl, NodeRangeT<ParameterDecl> params) {
    ostringstream oss;
    oss << decl.name() << "(";
    bool first = true;
    for (auto param : params) {
        if (!first)
            oss << ", ";
        first = false;

        StringRef name = param.name();
        NodeHandle typeNode = param.typeNode();
        Type type = typeNode ? tryGetTypeValue(typeNode) : Type();
        if (name)
            oss << name << ": ";
        if (type)
            oss << type;
        else
            oss << '?';
    }
    oss << ")";
    return oss.str();
}

} // namespace SprFrontend
