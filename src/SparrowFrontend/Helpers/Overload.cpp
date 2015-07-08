#include <StdInc.h>
#include "Overload.h"
#include "DeclsHelpers.h"
#include "Impl/FunctionCallable.h"
#include "Impl/GenericCallable.h"
#include "Impl/ClassCtorCallable.h"
#include "Impl/ConceptCallable.h"
#include <Helpers/DeclsHelpers.h>
#include <NodeCommonsCpp.h>
#include <Nodes/Decls/SprConcept.h>

#include <Feather/Nodes/ChangeMode.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Util/Decl.h>

#include <Nest/Common/ScopeGuard.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace
{
    /// Given a declaration, try to gets a list of Callable objects from it; returns an empty list if the declaration is not callable
    Callables getCallables(DynNode* decl, EvalMode evalMode)
    {
        Callables res;

        DynNode* resDecl = resultingDecl(decl);

        // Is this a normal function call?
        Function* fun = resDecl->as<Function>();
        if ( fun )
        {
            res.push_back(new FunctionCallable(fun));
            return res;
        }

        // Is this a generic?
        if ( isGeneric(resDecl) )
        {
            res.push_back(new GenericCallable(static_cast<Generic*>(resDecl)));
            return res;
        }

        // Is this a concept?
        SprConcept* concept = dynamic_cast<SprConcept*>(resDecl);
        if ( concept )
        {
            res.push_back(new ConceptCallable(concept));
            return res;
        }

        // Is this a temporary object creation?
        Class* cls = resDecl->as<Class>();
        if ( cls )
            return ClassCtorCallable::getCtorCallables(cls, evalMode);

        return res;
    }


    /// Filter candidates by checking the arguments against each candidate
    /// Retain only the candidates with the highest conversion
    /// Accepts either the argument nodes, or their type
    Callables filterCandidates(CompilationContext* context, const Location& loc, const Callables& candidates, const DynNodeVector* args, const vector<TypeRef>* argTypes, EvalMode evalMode, bool noCustomCvt = false)
    {
        Callables res;
        ConversionType bestConv = convNone;
        res.reserve(candidates.size());
        for ( Callable* cand: candidates )
        {
            ConversionType conv = args ? cand->canCall(context, loc, *args, evalMode, noCustomCvt) : cand->canCall(context, loc, *argTypes, evalMode, noCustomCvt);
            if ( conv == convNone )
                continue;

            if ( conv > bestConv )
            {
                bestConv = conv;
                res.clear();
            }
            if ( conv == bestConv )
                res.push_back(cand);
        }
        return res;
    }

    /// Returns -1 if f1 is more specialized than f2, 1 if f2 is more specialized than f1; or 0 if neighter is more specialized
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

    Callable* selectMostSpecialized(CompilationContext* context, const Callables& candidates, bool noCustomCvt = false)
    {
        if ( candidates.empty() )
            return nullptr;
        if ( candidates.size() == 1 )
            return candidates.front();

        // Check which function is most specialized
        for ( size_t i=0; i<candidates.size(); ++i )
        {
            bool isMostSpecialized = true;
            for ( size_t j=i+1; j<candidates.size(); ++j )
            {
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

    string nameWithAguments(const string funName, const vector<TypeRef>& argsTypes)
    {
        ostringstream oss;
        oss << funName;
        oss << "(";
        for ( size_t i=0; i<argsTypes.size(); ++i )
        {
            if ( i>0 )
                oss << ", ";
            oss << argsTypes[i];
        }
        oss << ")";
        return oss.str();
    }

    void doReportErrors(const Location& loc, const DynNodeVector& decls, const Callables& candidates,
        const vector<TypeRef>& argsTypes, const string& funName)
    {
        REP_ERROR_NOTHROW(loc, "No matching overload found for calling %1%") % nameWithAguments(funName, argsTypes);
        for ( Callable* cand: candidates )
        {
            REP_INFO(cand->location(), "See possible candidate: %1%") % cand->toString();
        }
        if ( candidates.empty() )
        {
            for ( DynNode* decl: decls )
            {
                REP_INFO(decl->location(), "See possible candidate: %1%") % decl->toString();
            }
        }
        REP_ERROR_THROW("NoOverloadFound");
    }
}

DynNode* SprFrontend::selectOverload(CompilationContext* context, const Location& loc, Nest::EvalMode evalMode,
        DynNodeVector decls, DynNodeVector args,
        bool reportErrors, const string& funName)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "others.overload", "Overloading selection");

    // Special case for macro calls
    bool isMacro = decls.size() == 1 && decls[0]->hasProperty(propMacro);
    if ( isMacro )
    {
        // Wrap every argument in a lift(...) call
        for ( auto& arg: args )
        {
            const Location& l = arg->location();
            arg = mkFunApplication(l, mkIdentifier(l, "lift"), mkNodeList(l, {arg}, true));
            arg->setContext(context);
        }
    }

    vector<TypeRef> argsTypes(args.size(), nullptr);
    for ( size_t i=0; i<args.size(); ++i)
    {
        args[i]->semanticCheck();
        argsTypes[i] = args[i]->type();
    }

    if ( decls.empty() )
    {
        if ( reportErrors )
            doReportErrors(loc, decls, Callables(), argsTypes, funName);
        return nullptr;
    }

    // If the desired eval-mode is different from the context's mode, create a new context
    // We do this by wrapping everything inside a ChangeMode node
    ChangeMode* changeModeNode = nullptr;
    if ( context->evalMode() != evalMode )
    {
        changeModeNode = new ChangeMode(loc, evalMode);
        changeModeNode->setContext(context);
        context = changeModeNode->childrenContext();
    }

    // First, get all the candidates
    Callables candidates1;
    auto guard1 = Nest::Common::makeGuard([&]()-> void { destroyCallables(candidates1); });
    candidates1.reserve(decls.size());
    for ( DynNode* decl: decls )
    {
        decl->computeType();
        auto newCandidates = getCallables(decl, evalMode);
        candidates1.insert(candidates1.end(), newCandidates.begin(), newCandidates.end());
    }
    if ( candidates1.empty() )
    {
        if ( reportErrors )
            doReportErrors(loc, decls, Callables(), argsTypes, funName);
        return nullptr;
    }

    // Check the candidates to be able to be called with the given arguments
    Callables candidates = filterCandidates(context, loc, candidates1, &args, nullptr, evalMode);
    auto guard = Nest::Common::makeGuard([&]()-> void { destroyCallables(candidates1); });
    if ( candidates.empty() )
    {
        if ( reportErrors )
            doReportErrors(loc, decls, candidates1, argsTypes, funName);
        return nullptr;
    }

    // From the remaining candidates, try to select the most specialized one
    Callable* selectedFun = selectMostSpecialized(context, candidates);
    if ( !selectedFun )
    {
        if ( reportErrors )
            doReportErrors(loc, decls, candidates, argsTypes, funName);
        return nullptr;
    }

    DynNode* res = selectedFun->generateCall(loc);
    ASSERT(res->context());
    if ( changeModeNode )
    {
        changeModeNode->setChild(res);
        res = changeModeNode;
    }


    if ( isMacro )
    {
        // Wrap the function call in a Meta.astEval(...) call
        DynNode* funName = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "astEval");
        res = mkFunApplication(loc, funName, { res });
        res->setContext(context);
    }

    return res;
}

bool SprFrontend::selectConversionCtor(CompilationContext* context, Class* destClass, EvalMode destMode,
        TypeRef argType, DynNode* arg, DynNode** conv)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "others.selCvtCtor", "Selecting conversion ctor");

    ASSERT(argType);

    // Search for the ctors in the class 
    DynNodeVector decls = destClass->childrenContext()->currentSymTab()->lookupCurrent("ctor");

//     cerr << "Convert: " << argType->toString() << " -> " << destClass->toString() << " ?" << endl;

    // Get all the candidates
    Callables candidates;
    candidates.reserve(decls.size());
    for ( DynNode* decl: decls )
    {
        if ( !decl->hasProperty(propConvert) )
            continue;

        decl->computeType();
        DynNode* resDecl = resultingDecl(decl);

        Callables callables = getCallables(resDecl, destMode);
        for ( Callable* c: callables )
        {
            candidates.push_back(new ClassCtorCallable(destClass, c, destMode));
        }
    }
    if ( candidates.empty() )
        return false;

    // Check the candidates to be able to be called with the given arguments
    vector<TypeRef> argTypes(1, argType);
    candidates = filterCandidates(context, arg ? arg->location() : Location(), candidates, nullptr, &argTypes, destMode, true);
    if ( candidates.empty() )
        return false;

    // From the remaining candidates, try to select the most specialized one
    Callable* selectedFun = selectMostSpecialized(context, candidates, true);
    if ( !selectedFun )
        return false;

//     cerr << "SUCCESS!!!" << endl;
    if ( arg && conv )
    {
        arg->computeType();
        auto cr = selectedFun->canCall(context, arg->location(), { arg }, destMode, true);
        (void) cr;
        ASSERT(cr);
        *conv = selectedFun->generateCall(arg->location());
        (*conv)->setContext(context);
        (*conv)->semanticCheck();
    }
    return true;
}

Callable* SprFrontend::selectCtToRtCtor(CompilationContext* context, TypeRef ctType)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "others.selCtRtCtor", "Selecting ct-to-rt ctor");

    if ( ctType->mode != modeCt || !ctType->hasStorage )
        return nullptr;
    Class* cls = Feather::classDecl(ctType);
    if ( effectiveEvalMode(cls) != modeRtCt )
        return nullptr;

    // Search for the ctors in the class 
    DynNodeVector decls = cls->childrenContext()->currentSymTab()->lookupCurrent("ctorFromCt");

    // Select the possible ct-to-rt constructors
    Callables candidates;
    candidates.reserve(decls.size());
    for ( DynNode* decl: decls )
    {
        if ( effectiveEvalMode(decl) != modeRt )
            continue;

        decl->computeType();
        DynNode* resDecl = resultingDecl(decl);
        ASSERT(effectiveEvalMode(resDecl) == modeRt);

        Callables callables = getCallables(resDecl, modeRt);
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
    if ( candidates.empty() )
        return nullptr;

    // Check the candidates to be able to be called with the given arguments
    vector<TypeRef> argTypes(1, ctType);
    candidates = filterCandidates(context, Location(), candidates, nullptr, &argTypes, modeRt, true);
    if ( candidates.empty() )
        return nullptr;

    // From the remaining candidates, try to select the most specialized one
    return selectMostSpecialized(context, candidates, true);
}
