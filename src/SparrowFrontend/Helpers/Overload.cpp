#include <StdInc.h>
#include "Overload.h"
#include "DeclsHelpers.h"
#include "Impl/FunctionCallable.h"
#include "Impl/GenericCallable.h"
#include "Impl/ClassCtorCallable.h"
#include "Impl/ConceptCallable.h"
#include <Helpers/DeclsHelpers.h>
#include <Helpers/Generics.h>
#include <NodeCommonsCpp.h>

#include "Feather/Utils/FeatherUtils.hpp"

using namespace SprFrontend;
using namespace Nest;

namespace
{
    /// Called at the start of error reporting
    /// Reports a general error if we are in 'full' error reporting mode;
    /// otherwise just reports an info message
    void startError(OverloadReporting errReporting, const Location& loc,
        const vector<TypeRef>& argsTypes, StringRef funName)
    {
        // Compute function name with arguments
        ostringstream oss;
        oss << funName.begin;
        oss << "(";
        for ( size_t i=0; i<argsTypes.size(); ++i ) {
            if ( i>0 )
                oss << ", ";
            oss << argsTypes[i];
        }
        oss << ")";

        if ( errReporting == OverloadReporting::full )
            REP_ERROR(loc, "No matching overload found for calling %1%") % oss.str();
        else
            REP_INFO(NOLOC, "No matching overload found for calling %1%") % oss.str();
    }

    /// Report the decls used for selecting our call candidates
    /// This is called whenever we cannot generate callables our of these decls
    void reportDeclsAlternatives(const Location& loc, NodeRange decls)
    {
        REP_INFO(NOLOC, "Reason: No callable declaration found");
        for ( Node* decl: decls ) {
            REP_INFO(decl->location, "See non-callable declaration: %1%") % decl;
        }
    }

    /// Given a declaration, try to gets a list of Callable objects from it.
    /// Returns an empty list if the declaration is not callable
    void getCallables(NodeRange decls, EvalMode evalMode, Callables& res)
    {
        NodeArray declsEx = expandDecls(decls, nullptr);

        for ( Node* decl: declsEx ) {
            Node* node = decl;

            // If we have a resolved decl, get the callable for it
            if ( node ) {
                if ( !Nest_computeType(node) )
                    continue;

                Node* decl = resultingDecl(node);

                // Is this a normal function call?
                if ( decl && decl->nodeKind == nkFeatherDeclFunction )
                    res.push_back(new FunctionCallable(decl));

                // Is this a generic?
                else if ( isGeneric(decl) )
                    res.push_back(new GenericCallable(decl));

                // Is this a concept?
                else if ( decl->nodeKind == nkSparrowDeclSprConcept )
                    res.push_back(new ConceptCallable(decl));

                // Is this a temporary object creation?
                else {
                    Node* cls = decl && decl->nodeKind == nkFeatherDeclClass ? decl : nullptr;
                    if ( cls ) {
                        auto r1 = ClassCtorCallable::getCtorCallables(cls, evalMode);
                        res.insert(res.end(), r1.begin(), r1.end());
                    }
                }
            }
        }
    }


    /// Filter candidates by checking the arguments against each candidate
    /// Retain only the candidates with the highest conversion
    /// Accepts either the argument nodes, or their type
    /// Returns true if there are some valid candidates
    bool filterCandidates(CompilationContext* context, const Location& loc, const Callables& candidates, NodeRange* args, const vector<TypeRef>* argTypes, EvalMode evalMode, bool noCustomCvt = false)
    {
        ConversionType bestConv = convNone;
        for ( size_t i=0; i<candidates.size(); ++i ) {
            Callable* cand = candidates[i];

            ConversionType conv = args
                                    ? cand->canCall(context, loc, *args, evalMode, noCustomCvt)
                                    : cand->canCall(context, loc, *argTypes, evalMode, noCustomCvt);
            if ( conv == convNone ) {
                cand->markInvalid();
                continue;
            }

            if ( conv > bestConv ) {
                bestConv = conv;
                for ( size_t j = 0; j<i; ++j )
                    if ( candidates[j]->isValid() ) {
                        candidates[j]->markInvalid();
                    }
            }
            else if ( conv < bestConv )
                cand->markInvalid();
        }
        return bestConv != convNone;
    }

    /// This is called if filterCandidates failed to select any valid candidate.
    /// This will report all the candidates, and why they could not be called.
    void filterCandidatesErrReport(CompilationContext* context, const Location& loc, const Callables& candidates, NodeRange* args, const vector<TypeRef>* argTypes, EvalMode evalMode, bool noCustomCvt = false)
    {
        for ( size_t i=0; i<candidates.size(); ++i ) {
            Callable* cand = candidates[i];

            // Report the candidate
            REP_INFO(cand->location(), "See possible candidate: %1%") % cand->toString();

            if ( args )
                cand->canCall(context, loc, *args, evalMode, noCustomCvt, true);
            else
                cand->canCall(context, loc, *argTypes, evalMode, noCustomCvt, true);
        }
    }

    /// Returns who of two candidates is more specialized.
    /// Returns:
    ///     -1 if f1 is more specialized than f2,
    ///     1 if f2 is more specialized than f1;
    ///     0 if neighter is more specialized
    int moreSpecialized(CompilationContext* context, Callable* f1, Callable* f2, bool noCustomCvt = false)
    {
        // Check parameter count
        size_t paramsCount = f1->paramsCount();
        if ( paramsCount != f2->paramsCount() )
            return 0;

        bool firstIsMoreSpecialized = false;
        bool secondIsMoreSpecialized = false;
        for ( size_t i=0; i<paramsCount; ++i )
        {
            TypeRef t1 = f1->paramType(i);
            TypeRef t2 = f2->paramType(i);

            // Ignore parameters of same type
            if ( t1 == t2 )
                continue;

            ConversionFlags flags = noCustomCvt ? flagDontCallConversionCtor : flagsDefault;
            ConversionResult c1 = canConvertType(context, t1, t2, flags);
            if ( c1 )
            {
                firstIsMoreSpecialized = true;
                if ( secondIsMoreSpecialized )
                    return 0;
            }
            ConversionResult c2 = canConvertType(context, t2, t1, flags);
            if ( c2 )
            {
                secondIsMoreSpecialized = true;
                if ( firstIsMoreSpecialized )
                    return 0;
            }
        }
        if ( firstIsMoreSpecialized && !secondIsMoreSpecialized )
            return -1;
        else if ( !firstIsMoreSpecialized && secondIsMoreSpecialized )
            return 1;
        else
            return 0;
    }

    /// Selects the most specialized callable from a list of callables.
    /// If there is not such thing as a "most specialized", this will return null.
    Callable* selectMostSpecialized(CompilationContext* context, const Callables& candidates, bool noCustomCvt = false)
    {
        if ( candidates.empty() )
            return nullptr;

        // Check if we have only one valid candidate
        // If so, just return it
        Callable* oneCand = nullptr;
        for ( Callable* cand : candidates ) {
            if ( cand->isValid() ) {
                if ( oneCand ) {
                    // More than one valid candidate
                    oneCand = nullptr;
                    break;
                }
                else
                    oneCand = cand;
            }
        }
        if ( oneCand )
            return oneCand;

        // Check which function is most specialized
        for ( size_t i=0; i<candidates.size(); ++i )
        {
            if ( !candidates[i]->isValid() )
                continue;
            bool isMostSpecialized = true;
            for ( size_t j=0; j<candidates.size(); ++j )
            {
                if ( j ==i || !candidates[j]->isValid() )
                    continue;
                int res = moreSpecialized(context, candidates[i], candidates[j], noCustomCvt);
                // if res < 0, the current function is more specialized than the other function
                if ( res >= 0 )
                {
                    isMostSpecialized = false;
                    break;
                }
            }
            if ( isMostSpecialized )
                return candidates[i];
        }
        return nullptr;
    }

    /// Called when 'selectMostSpecialized' failed to select a candidate.
    /// This will report the valid candidates that were taken into consideration
    /// for 'most specialized' selection
    void selectMostSpecializedErrReport(CompilationContext* context, const Callables& candidates, bool noCustomCvt = false)
    {
        // Assume we have more than one valid candidate

        REP_INFO(NOLOC, "Reason: Cannot select a 'most specialized' function");

        for ( Callable* cand : candidates ) {
            if ( cand && cand->isValid() ) {
                REP_INFO(cand->location(), "See valid candidate: %1%") % cand->toString();
            }
        }
    }
}

Node* SprFrontend::selectOverload(CompilationContext* context, const Location& loc, EvalMode evalMode,
        NodeRange decls, NodeRange args,
        OverloadReporting errReporting, StringRef funName)
{
    auto numDecls = Nest_nodeRangeSize(decls);
    Node* firstDecl = numDecls > 0 ? at(decls, 0) : nullptr;

    // Special case for macro calls
    bool isMacro = firstDecl && Nest_hasProperty(firstDecl, propMacro);
    if ( isMacro )
    {
        // Wrap every argument in an astLift(...) call
        for ( auto& arg: args )
        {
            const Location& l = arg->location;
            arg = mkFunApplication(l, mkIdentifier(l, fromCStr("astLift")), Feather_mkNodeListVoid(l, fromIniList({ arg })));
            Nest_setContext(arg, context);
        }
    }

    // Computing the argument types for our arguments
    auto numArgs = Nest_nodeRangeSize(args);
    vector<TypeRef> argsTypes(numArgs, nullptr);
    for ( size_t i=0; i<numArgs; ++i)
    {
        Node* arg = at(args, i);
        if ( !Nest_semanticCheck(arg) )
            return nullptr;
        argsTypes[i] = arg->type;
    }

    if ( numDecls == 0 )
    {
        if ( errReporting != OverloadReporting::none ) {
            startError(errReporting, loc, argsTypes, funName);
            REP_INFO(NOLOC, "Reason: No declarations found for %1%") % funName;
        }
        return nullptr;
    }

    // If the desired eval-mode is different from the context's mode, create a new context
    // We do this by wrapping everything inside a ChangeMode node
    Node* changeModeNode = nullptr;
    if ( context->evalMode != evalMode )
    {
        changeModeNode = Feather_mkChangeMode(loc, nullptr, evalMode);
        Nest_setContext(changeModeNode, context);
        context = Nest_childrenContext(changeModeNode);
    }

    // First, get all the candidates
    Callables candidates;
    CallablesDestroyer candidatesDestoyer(candidates);
    candidates.reserve(numDecls*2);
    getCallables(decls, evalMode, candidates);
    if ( candidates.empty() )
    {
        if ( errReporting != OverloadReporting::none ) {
            startError(errReporting, loc, argsTypes, funName);
            reportDeclsAlternatives(loc, decls);
        }
        return nullptr;
    }

    // Check the candidates to be able to be called with the given arguments
    bool hasValidCandidates = filterCandidates(context, loc, candidates, &args, nullptr, evalMode);
    if ( !hasValidCandidates )
    {
        if ( errReporting != OverloadReporting::none ) {
            startError(errReporting, loc, argsTypes, funName);
            filterCandidatesErrReport(context, loc, candidates, &args, nullptr, evalMode);
        }
        return nullptr;
    }

    // From the remaining candidates, try to select the most specialized one
    Callable* selectedFun = selectMostSpecialized(context, candidates);
    if ( !selectedFun )
    {
        if ( errReporting != OverloadReporting::none ) {
            startError(errReporting, loc, argsTypes, funName);
            selectMostSpecializedErrReport(context, candidates);
        }
        return nullptr;
    }

    // Generate the call code for the selected fun
    Node* res = selectedFun->generateCall(loc);
    if ( !res ) {
        if ( errReporting != OverloadReporting::none ) {
            startError(errReporting, loc, argsTypes, funName);
            REP_ERROR(selectedFun->location(), "Cannot generate call code for selected overload: %1%")
                % selectedFun->toString();
        }
        return nullptr;
    }
    ASSERT(res->context);
    if ( changeModeNode )
    {
        Feather_ChangeMode_setChild(changeModeNode, res);
        res = changeModeNode;
    }


    if ( isMacro )
    {
        // Wrap the function call in a astEval(...) call
        Node* qid = mkIdentifier(loc, fromCStr("astEval"));
        res = mkFunApplication(loc, qid, fromIniList({res}));
        Nest_setContext(res, context);
    }

    return res;
}

bool SprFrontend::selectConversionCtor(CompilationContext* context, Node* destClass, EvalMode destMode,
        TypeRef argType, Node* arg, Node** conv)
{
    ASSERT(argType);

    // Search for the ctors in the class
    NodeArray decls = Nest_symTabLookupCurrent(Nest_childrenContext(destClass)->currentSymTab, "ctor");

//     cerr << "Convert: " << argType->toString() << " -> " << Nest_toString(destClass) << " ?" << endl;

    // Get all the candidates
    Callables candidates;
    candidates.reserve(Nest_nodeArraySize(decls));
    for ( Node* decl: decls )
    {
        if ( !Nest_hasProperty(decl, propConvert) )
            continue;

        if ( !Nest_computeType(decl) )
            continue;
        Node* resDecl = resultingDecl(decl);

        Callables callables;
        getCallables(fromIniList({resDecl}), destMode, callables);
        for ( Callable* c: callables )
        {
            candidates.push_back(new ClassCtorCallable(destClass, c, destMode));
        }
    }
    Nest_freeNodeArray(decls);
    if ( candidates.empty() )
        return false;

    // Check the candidates to be able to be called with the given arguments
    vector<TypeRef> argTypes(1, argType);
    filterCandidates(context, arg ? arg->location : Location(), candidates, nullptr, &argTypes, destMode, true);

    // From the remaining candidates, try to select the most specialized one
    Callable* selectedFun = selectMostSpecialized(context, candidates, true);
    if ( !selectedFun )
        return false;

//     cerr << "SUCCESS!!!" << endl;
    if ( arg && conv )
    {
        if ( !Nest_computeType(arg) )
            return false;
        auto cr = selectedFun->canCall(context, arg->location, fromIniList({ arg }), destMode, true);
        (void) cr;
        ASSERT(cr);
        *conv = selectedFun->generateCall(arg->location);
        Nest_setContext(*conv, context);
        Nest_semanticCheck(*conv);
    }
    return true;
}

Callable* SprFrontend::selectCtToRtCtor(CompilationContext* context, TypeRef ctType)
{
    if ( ctType->mode != modeCt || !ctType->hasStorage )
        return nullptr;
    Node* cls = Feather_classDecl(ctType);
    if ( Feather_effectiveEvalMode(cls) != modeRtCt )
        return nullptr;

    // Search for the ctors in the class
    NodeArray decls = Nest_symTabLookupCurrent(Nest_childrenContext(cls)->currentSymTab, "ctorFromCt");

    // Select the possible ct-to-rt constructors
    Callables candidates;
    candidates.reserve(Nest_nodeArraySize(decls));
    for ( Node* decl: decls )
    {
        if ( Feather_effectiveEvalMode(decl) != modeRt )
            continue;

        if ( !Nest_computeType(decl) )
            continue;
        Node* resDecl = resultingDecl(decl);
        ASSERT(Feather_effectiveEvalMode(resDecl) == modeRt);

        Callables callables;
        getCallables(fromIniList({resDecl}), modeRt, callables);
        for ( Callable* c: callables )
        {
            // We expect two parameters (this + arg); the second one should be ct
            if ( c->paramsCount() != 2 )
                continue;
            TypeRef t = c->paramType(1);
            if ( t->mode != modeCt )
                continue;

            ASSERT(c->evalMode() == modeRt);

            // We might have a match
            candidates.push_back(new ClassCtorCallable(cls, c, modeRt));
        }
    }
    Nest_freeNodeArray(decls);
    if ( candidates.empty() )
        return nullptr;

    // Check the candidates to be able to be called with the given arguments
    vector<TypeRef> argTypes(1, ctType);
    filterCandidates(context, Location(), candidates, nullptr, &argTypes, modeRt, true);

    // From the remaining candidates, try to select the most specialized one
    return selectMostSpecialized(context, candidates, true);
}
