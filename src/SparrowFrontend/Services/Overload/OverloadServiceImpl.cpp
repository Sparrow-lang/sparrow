#include "StdInc.h"
#include "SparrowFrontend/Services/Overload/OverloadServiceImpl.h"
#include "SparrowFrontend/Services/Callable/Callable.h"
#include "SparrowFrontend/Services/ICallableService.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/CommonCode.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "SparrowFrontend/SparrowFrontendTypes.hpp"
#include "SparrowFrontend/NodeCommonsCpp.h"
#include "SparrowFrontend/SprDebug.h"

#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using namespace SprFrontend;
using namespace Nest;

namespace SprFrontend {

namespace {

/// Called at the start of error reporting
/// Reports a general error if we are in 'full' error reporting mode;
/// otherwise just reports an info message
void startError(OverloadReporting errReporting, const Location& loc, const vector<Type>& argsTypes,
        StringRef funName) {
    // Compute function name with arguments
    ostringstream oss;
    oss << funName.begin;
    oss << "(";
    for (size_t i = 0; i < argsTypes.size(); ++i) {
        if (i > 0)
            oss << ", ";
        oss << argsTypes[i];
    }
    oss << ")";

    if (errReporting == OverloadReporting::full)
        REP_ERROR(loc, "No matching overload found for calling %1%") % oss.str();
    else
        REP_INFO(NOLOC, "No matching overload found for calling %1%") % oss.str();
}

/// Report the decls used for selecting our call candidates
/// This is called whenever we cannot generate callables our of these decls
void reportDeclsAlternatives(const Location& loc, Nest_NodeRange decls) {
    REP_INFO(NOLOC, "Reason: No callable declaration found");
    for (Node* decl : decls) {
        REP_INFO(decl->location, "See non-callable declaration: %1%") % decl;
    }
}

/// Filter candidates by checking the arguments against each candidate
/// Retain only the candidates with the highest conversion
/// Accepts either the argument nodes, or their type
/// Returns true if there are some valid candidates
/// If there are callables with smaller overload prio, drop them in the favor of those with higher
/// overload prio.
bool filterCandidates(CompilationContext* context, const Location& loc, Callables& candidates,
        Nest_NodeRange* args, Range<Type> argTypes, EvalMode evalMode,
        CustomCvtMode customCvtMode) {
    ConversionType bestConv = convNone;
    int bestPrio = INT_MIN;
    for (size_t i = 0; i < candidates.size(); ++i) {
        Callable* cand = candidates[i];

        // Check if this can be called with the given args
        ConversionType conv =
                args ? cand->canCall(CCLoc{context, loc}, *args, evalMode, customCvtMode)
                     : cand->canCall(CCLoc{context, loc}, argTypes, evalMode, customCvtMode);
        if (conv == convNone) {
            cand->invalidate();
            continue;
        }

        int prio = Nest_getPropertyDefaultInt(cand->decl(), propOverloadPrio, 0);

        if (prio > bestPrio || (prio == bestPrio && conv > bestConv)) {
            bestConv = conv;
            bestPrio = prio;
            for (size_t j = 0; j < i; ++j)
                if (candidates[j]->valid()) {
                    candidates[j]->invalidate();
                }
        } else if (prio < bestPrio || conv < bestConv)
            cand->invalidate();
    }

    return bestConv != convNone;
}

/// This is called if filterCandidates failed to select any valid candidate.
/// This will report all the candidates, and why they could not be called.
void filterCandidatesErrReport(CompilationContext* context, const Location& loc,
        Callables& candidates, Nest_NodeRange* args, Range<Type> argTypes,
        EvalMode evalMode, CustomCvtMode customCvtMode) {
    for (auto& cand : candidates) {
        // Report the candidate
        REP_INFO(cand->location(), "See possible candidate: %1%") % cand->toString();

        if (args)
            cand->canCall(CCLoc{context, loc}, *args, evalMode, customCvtMode, true);
        else
            cand->canCall(CCLoc{context, loc}, argTypes, evalMode, customCvtMode, true);
    }
}

/// Selects the most specialized callable from a list of callables.
/// If there is not such thing as a "most specialized", this will return null.
Callable* selectMostSpecialized(
        CompilationContext* context, Callables& candidates, bool noCustomCvt = false) {
    if (candidates.empty())
        return nullptr;

    // Check if we have only one valid candidate
    // If so, just return it
    Callable* oneCand = nullptr;
    for (Callable* cand : candidates) {
        if (cand->valid()) {
            if (oneCand) {
                // More than one valid candidate
                oneCand = nullptr;
                break;
            } else
                oneCand = cand;
        }
    }
    if (oneCand)
        return oneCand;

    // Check which function is most specialized
    for (size_t i = 0; i < candidates.size(); ++i) {
        if (!candidates[i]->valid())
            continue;
        bool isMostSpecialized = true;
        for (size_t j = 0; j < candidates.size(); ++j) {
            if (j == i || !candidates[j]->valid())
                continue;
            int res = moreSpecialized(context, *candidates[i], *candidates[j], noCustomCvt);
            // if res < 0, the current function is more specialized than the
            // other function
            if (res >= 0) {
                isMostSpecialized = false;
                break;
            }
        }
        if (isMostSpecialized)
            return candidates[i];
    }
    return nullptr;
}

/// Called when 'selectMostSpecialized' failed to select a candidate.
/// This will report the valid candidates that were taken into consideration
/// for 'most specialized' selection
void selectMostSpecializedErrReport(
        CompilationContext* context, const Callables& candidates, bool noCustomCvt = false) {
    // Assume we have more than one valid candidate

    REP_INFO(NOLOC, "Reason: Cannot select a 'most specialized' function");

    for (const Callable* cand : candidates) {
        if (cand->valid()) {
            REP_INFO(cand->location(), "See valid candidate: %1%") % cand->toString();
            // printNode(cand.decl);
        }
    }
}
} // namespace

Node* OverloadServiceImpl::selectOverload(CompilationContext* context, const Location& loc,
        EvalMode evalMode, Nest_NodeRange decls, Nest_NodeRange args,
        OverloadReporting errReporting, StringRef funName) {
    auto numDecls = Nest_nodeRangeSize(decls);
    Node* firstDecl = numDecls > 0 ? at(decls, 0) : nullptr;

    // Special case for macro calls
    bool isMacro = firstDecl && Nest_hasProperty(firstDecl, propMacro);
    NodeVector newArgs;
    if (isMacro) {
        newArgs = NodeVector(args.beginPtr, args.endPtr);
        // Wrap every argument in an astLift(...) call
        for (auto& arg : newArgs) {
            const Location& l = arg->location;
            arg = mkFunApplication(l, mkIdentifier(l, StringRef("astLift")),
                    Feather_mkNodeListVoid(l, fromIniList({arg})));
            Nest_setContext(arg, context);
        }
        args = all(newArgs);
    }

    // Computing the argument types for our arguments
    auto numArgs = Nest_nodeRangeSize(args);
    vector<Type> argsTypes(numArgs, nullptr);
    for (size_t i = 0; i < numArgs; ++i) {
        Node* arg = at(args, i);
        ASSERT(arg);
        if (!Nest_semanticCheck(arg))
            return nullptr;
        argsTypes[i] = arg->type;
    }

    if (numDecls == 0) {
        if (errReporting != OverloadReporting::none) {
            startError(errReporting, loc, argsTypes, funName);
            REP_INFO(NOLOC, "Reason: No declarations found for %1%") % funName;
        }
        return nullptr;
    }

    // If the desired eval-mode is different from the context's mode, create a
    // new context
    // We do this by wrapping everything inside a ChangeMode node
    Node* changeModeNode = nullptr;
    if (context->evalMode != evalMode) {
        changeModeNode = Feather_mkChangeMode(loc, nullptr, evalMode);
        Nest_setContext(changeModeNode, context);
        context = Nest_childrenContext(changeModeNode);
    }

    // First, get all the candidates
    Callables candidates = g_CallableService->getCallables(decls, evalMode);
    if (candidates.empty()) {
        if (errReporting != OverloadReporting::none) {
            startError(errReporting, loc, argsTypes, funName);
            reportDeclsAlternatives(loc, decls);
        }
        return nullptr;
    }

    // For ctors and dtors don't allow custom conversions for the first
    // parameter
    CustomCvtMode customCvtMode = allowCustomCvt;
    if (funName == "ctor" || funName == "dtor")
        customCvtMode = noCustomCvtForFirst;

    // Check the candidates to be able to be called with the given arguments
    bool hasValidCandidates =
            filterCandidates(context, loc, candidates, &args, {}, evalMode, customCvtMode);
    if (!hasValidCandidates) {
        if (errReporting != OverloadReporting::none) {
            startError(errReporting, loc, argsTypes, funName);
            filterCandidatesErrReport(
                    context, loc, candidates, &args, {}, evalMode, customCvtMode);
        }
        return nullptr;
    }

    // From the remaining candidates, try to select the most specialized one
    Callable* selectedFun = selectMostSpecialized(context, candidates);
    if (!selectedFun) {
        if (errReporting != OverloadReporting::none) {
            startError(errReporting, loc, argsTypes, funName);
            selectMostSpecializedErrReport(context, candidates);
        }
        return nullptr;
    }

    // Generate the call code for the selected fun
    Node* res = selectedFun->generateCall(CCLoc{context, loc});
    if (!res) {
        if (errReporting != OverloadReporting::none) {
            startError(errReporting, loc, argsTypes, funName);
            REP_INFO(NOLOC, "Cannot generate call code for selected overload: %1%") %
                    selectedFun->toString();
        }
        return nullptr;
    }
    ASSERT(res->context);
    if (changeModeNode) {
        Feather::ChangeMode(changeModeNode).setChild(res);
        res = changeModeNode;
    }

    if (isMacro) {
        // Wrap the function call in a astEval(...) call
        Node* qid = mkIdentifier(loc, StringRef("astEval"));
        res = mkFunApplication(loc, qid, fromIniList({res}));
        Nest_setContext(res, context);
    }

    return res;
}

bool OverloadServiceImpl::selectConversionCtor(
        CompilationContext* context, Node* destClass, EvalMode destMode, Type argType) {
    ASSERT(argType);

    // Get all the candidates
    Callables candidates = g_CallableService->getCallables(fromIniList({destClass}), destMode,
            [](NodeHandle decl) -> bool { return decl.hasProperty(propConvert); });
    if (candidates.empty())
        return false;

    // Check the candidates to be able to be called with the given arguments
    vector<Type> argTypes(1, argType);
    filterCandidates(context, Location(), candidates, nullptr, argTypes, destMode, noCustomCvt);

    // From the remaining candidates, try to select the most specialized one
    Callable* selectedFun = selectMostSpecialized(context, candidates, true);
    if (!selectedFun)
        return false;

    return true;
}

Node* OverloadServiceImpl::selectCtToRtCtor(Node* ctArg) {
    const Location& loc = ctArg->location;
    ASSERT(ctArg->type);
    if (ctArg->type->mode != modeCt || !ctArg->type->hasStorage)
        return nullptr;
    Node* cls = Feather_classDecl(ctArg->type);
    if (Feather_effectiveEvalMode(cls) != modeRt)
        return nullptr;

    // Select the possible ct-to-rt constructors
    Callables candidates =
            g_CallableService->getCallables(fromIniList({cls}), modeRt, {}, "ctorFromCt");
    if (candidates.empty())
        return nullptr;

    // Check the candidates to be able to be called with the given arguments
    vector<Type> argTypes(1, ctArg->type);
    bool hasValidCandidates = filterCandidates(
            ctArg->context, Location(), candidates, nullptr, argTypes, modeRt, noCustomCvt);
    if (!hasValidCandidates) {
        REP_ERROR(loc, "No matching overload found for calling ctorFromCt");
        filterCandidatesErrReport(
                ctArg->context, loc, candidates, nullptr, argTypes, modeRt, noCustomCvt);
        return nullptr;
    }

    // From the remaining candidates, try to select the most specialized one
    Callable* call = selectMostSpecialized(ctArg->context, candidates, true);
    if (!call) {
        REP_ERROR(loc, "No matching overload found for calling ctorFromCt");
        selectMostSpecializedErrReport(ctArg->context, candidates);
        return nullptr;
    }

    // Generate the call to the ctor
    auto cr = call->canCall(CCLoc{ctArg->context, loc}, fromIniList({ctArg}), modeRt, noCustomCvt);
    ASSERT(cr);
    if (!cr)
        return nullptr;
    return call->generateCall(CCLoc{ctArg->context, loc});
}

} // namespace SprFrontend
