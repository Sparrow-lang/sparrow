#include <StdInc.h>
#include "GenericFunction.h"
#include "Instantiation.h"
#include "SprFunction.h"
#include <Helpers/StdDef.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/CommonCode.h>
#include <Helpers/DeclsHelpers.h>

#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;


namespace
{
    /// Get the bound arguments corresponding to the CT or Auto parameters of the generic
    /// We return here the CT values of these arguments; we use their value to check for duplicate instantiations
    DynNodeVector getBoundValues(CompilationContext* context, const DynNodeVector& args, const DynNodeVector& genericParams)
    {
        DynNodeVector boundValues;
        boundValues.resize(args.size(), nullptr);

        // There are two types of bound arguments:
        //  - parameters with CT types
        //  - auto parameters - in this case we retain the type of the argument as a bound value
        for ( size_t i=0; i<args.size(); ++i )
        {
            DynNode* arg = args[i];
            DynNode* param = genericParams[i];
            if ( !param )
                continue;

            bool isRefAuto;
            if ( isConceptType(param->type(), isRefAuto) )
            {
                // Create a CtValue with the type of the argument corresponding to the auto parameter
                arg->computeType();
                TypeRef t = getAutoType(arg, isRefAuto);
                DynNode* typeNode = createTypeNode(context, param->location(), t);
                typeNode->computeType();
                boundValues[i] = typeNode;
            }
            else
            {
                // Evaluate the node and add the resulting CtValue as a bound argument
                arg->computeType();
                if ( !Feather::isCt(arg->node()) )
                    return {};     // This argument must be CT in order to instantiate the generic
                DynNode* n = (DynNode*) theCompiler().ctEval(arg->node());
                if ( !n || n->nodeKind() != nkFeatherExpCtValue )
                    REP_INTERNAL(arg->location(), "Invalid argument %1% when instantiating generic") % (i+1);
                boundValues[i] = n;
            }
        }
        return boundValues;
    }

    /// Get the list of non-bound parameters (including auto parameters) to be passed to an instantiation
    /// There are two types of non-bound arguments:
    ///  - parameters that are not marked as generic (e.g., RT types)
    ///  - auto parameters - the type is considered bound, but the argument value is non-bound
    /// For auto parameters create a parameter with the auto-type corresponding to the bound type of the argument
    /// Do not return the this parameter.
    /// Does not return the original parameters; creates a clone if needed
    DynNodeVector getNonBoundParameters(Instantiation& inst, SprFunction* originalFun, const DynNodeVector& params, const DynNodeVector& genericParams)
    {
        const auto& boundValues = inst.boundValues();
        ASSERT(!boundValues.empty());
        DynNodeVector nonBoundParams;
        nonBoundParams.reserve(params.size());
        for ( size_t i=0; i<params.size(); ++i )
        {
            if ( i==0 && originalFun->hasThisParameters() )
                continue;

            DynNode* p = params[i];
            DynNode* boundValue = boundValues[i];

            if ( !genericParams[i] )            // If this is not a generic parameter => non-bound parameter
            {
                nonBoundParams.push_back(p->clone());
            }
            else if ( isConceptType(p->type()) )   // For auto-type parameters, we also create a non-bound parameter
            {
                nonBoundParams.push_back(mkSprParameter(p->location(), getName(p->node()), boundValue));
            }
        }
        return nonBoundParams;
    }


    /// Get the eval mode for the resulting function; check the eval mode of the original function, of the non-bound arguments, and of the types
    EvalMode getResultingEvalMode(const Location& loc, EvalMode mainEvalMode, const DynNodeVector& args, const DynNodeVector& genericParams)
    {
        bool hasRtOnlyArgs = false;
        bool hasCtOnlyArgs = false;
        ASSERT(args.size() == genericParams.size());
        for ( size_t i=0; i<genericParams.size(); ++i)
        {
            // Test auto and non-bound arguments
            // Also test the type given to the 'Type' parameters (i.e., we need to know if Vector(t) can be rtct based on the mode of t)
            TypeRef pType = genericParams[i] ? genericParams[i]->type() : nullptr;
            TypeRef typeToCheck = nullptr;
            if ( !pType || isConceptType(pType) )
            {
                args[i]->computeType();
                typeToCheck = args[i]->type();
            }
            else
            {
                // Is the argument a Type?
                typeToCheck = tryGetTypeValue(args[i]);
            }
            if ( typeToCheck )
            {
                if ( !typeToCheck->canBeUsedAtCt )
                    hasRtOnlyArgs = true;
                else if ( !typeToCheck->canBeUsedAtRt )
                    hasCtOnlyArgs = true;
            }
        }
        if ( hasCtOnlyArgs && hasRtOnlyArgs )
            REP_ERROR(loc, "Cannot instantiate generic with both RT-only and CT-only arguments");
        if ( mainEvalMode == modeCt && hasRtOnlyArgs )
            REP_ERROR(loc, "Cannot use RT-only arguments in a CT generic");
        if ( mainEvalMode == modeRt && hasCtOnlyArgs )
            REP_ERROR(loc, "Cannot use CT-only arguments in a RT generic");

        if ( hasCtOnlyArgs )
            return modeCt;
        if ( hasRtOnlyArgs )
            return modeRt;
        return mainEvalMode;
    }

    /// From the list of arguments passed at generic instantiation, filter only the ones corresponding to the non-bound
    /// parameters, the ones that are passed to the actual instantiation.
    /// Note that this also returns the arguments corresponding to the auto parameters
    DynNodeVector getNonBoundArgs(const DynNodeVector& args, const DynNodeVector& genericParams)
    {
        DynNodeVector nonBoundArgs;
        nonBoundArgs.reserve(args.size());
        for ( size_t i=0; i<args.size(); ++i )
        {
            DynNode* param = genericParams[i];
            if ( !param || isConceptType(param->type()) )     // Get non-generic and also parameters
            {
                nonBoundArgs.push_back(args[i]);
            }
        }
        return nonBoundArgs;
    }

    DynNode* createInstFn(CompilationContext* context, SprFunction* origFun, const DynNodeVector& nonBoundParams)
    {
        const Location& loc = origFun->location();

        //REP_INFO(loc, "Instantiating %1% with %2% params") % getName(origFun) % nonBoundParams.size();

        NodeList* parameters = (NodeList*) mkNodeList(loc, fromDyn(nonBoundParams));
        DynNode* returnType = origFun->returnType();
        DynNode* body = origFun->body();
        returnType = returnType ? returnType->clone() : nullptr;
        body = body ? body->clone() : nullptr;
        DynNode* newFun = mkSprFunction(loc, getName(origFun->node()), parameters, returnType, body);
        copyModifiersSetMode(origFun, newFun, context->evalMode());
        setShouldAddToSymTab(newFun->node(), false);
        newFun->setContext(context);

        //REP_INFO(loc, "Instantiated %1%") % newFun->toString();
        return newFun;
    }

    DynNode* createCallFn(const Location& loc, CompilationContext* context, DynNode* inst, const DynNodeVector& nonBoundArgs)
    {
        SprFunction* sprFun = static_cast<SprFunction*>(inst);
        sprFun->computeType();
        if ( !sprFun->resultingFun() )
            REP_ERROR(loc, "Cannot instantiate function generic %1%") % getName(inst->node());
        return createFunctionCall(loc, context, sprFun->resultingFun(), nonBoundArgs);
    }
}


GenericFunction::GenericFunction(SprFunction* originalFun, DynNodeVector params, DynNodeVector genericParams, DynNode* ifClause)
    : Generic(classNodeKind(), originalFun, move(genericParams), ifClause, publicAccess)
{
    setEvalMode(node(), effectiveEvalMode(originalFun->node()));
    data_.referredNodes.push_back(mkNodeList(data_.location, fromDyn(move(params))));
}

GenericFunction::~GenericFunction()
{
}

const DynNodeVector& GenericFunction::params() const
{
    ASSERT(data_.referredNodes.size() == 2);
    return reinterpret_cast<const DynNodeVector&>(data_.referredNodes[1]->children);
}


GenericFunction* GenericFunction::createGeneric(SprFunction* originalFun, NodeList* parameters, DynNode* ifClause, Class* thisClass)
{
    // If we are in a CT function, don't consider CT parameters
    bool inCtFun = effectiveEvalMode(originalFun->node()) == modeCt;
    // For CT-generics, we consider all the parameters to be generic parameters
    bool isCtGeneric = originalFun->hasProperty(propCtGeneric);

    // Check if we have some CT parameters
    ASSERT(parameters);
    const DynNodeVector& params = parameters->children();
    DynNodeVector ourParams(params.size(), nullptr);
    DynNodeVector genericParams(params.size(), nullptr);
    bool hasGenericParams = false;
    for ( size_t i=0; i<params.size(); ++i )
    {
        DynNode* param = params[i];
        param->computeType();
        ASSERT(param->type());

        ourParams[i] = param;
        if ( isConceptType(param->type()) )
        {
            genericParams[i] = param;
            hasGenericParams = true;
        }
        if ( (!inCtFun || isCtGeneric) && isCt(param->node()) )
        {
            genericParams[i] = param;
            hasGenericParams = true;
        }
    }

    if ( !hasGenericParams )
        return nullptr;

    // If a 'this' class is passed, add an extra parameter for this
    if ( thisClass )
    {
        TypeRef thisType = getDataType((Node*) thisClass, 1, effectiveEvalMode(originalFun->node()));
        DynNode* thisParam = mkSprParameter(originalFun->location(), "$this", thisType);
        thisParam->setContext(originalFun->childrenContext());
        thisParam->computeType();
        ourParams.insert(ourParams.begin(), thisParam);
        genericParams.insert(genericParams.begin(), nullptr);
    }

    // Actually create the generic
    GenericFunction* res = new GenericFunction(originalFun, move(ourParams), move(genericParams), ifClause);
    setEvalMode(res->node(), effectiveEvalMode(originalFun->node()));
    res->setContext(originalFun->context());
    return res;
}

size_t GenericFunction::paramsCount() const
{
    return params().size();
}

DynNode* GenericFunction::param(size_t idx) const
{
    return params()[idx];
}

Instantiation* GenericFunction::canInstantiate(const DynNodeVector& args)
{
    DynNode* originalFun = (DynNode*) data_.referredNodes[0];
    DynNodeVector boundValues = getBoundValues(originalFun->context(), args, genericParams());

    EvalMode resultingEvalMode = originalFun->hasProperty(propCtGeneric)
        ? modeCt        // If we have a CT generic, the resulting eval mode is always CT
        : getResultingEvalMode(originalFun->location(), effectiveEvalMode(originalFun->node()), args, genericParams());

    InstantiationsSet* instantiationsSet = (InstantiationsSet*) data_.children[0];
    return instantiationsSet->canInstantiate(boundValues, resultingEvalMode);
}

DynNode* GenericFunction::instantiateGeneric(const Location& loc, CompilationContext* context, const DynNodeVector& args, Instantiation* inst)
{
    ASSERT(inst);

    // If not already created, create the actual instantiation declaration
    DynNode* instantiatedDecl = inst->instantiatedDecl();
    NodeList* expandedInstantiation = inst->expandedInstantiation();
    if ( !instantiatedDecl )
    {
        SprFunction* originalFun = ((DynNode*) data_.referredNodes[0])->as<SprFunction>();
        DynNodeVector nonBoundParams = getNonBoundParameters(*inst, originalFun, params(), genericParams());

        // Create the actual instantiation declaration
        CompilationContext* ctx = expandedInstantiation->childrenContext();
        instantiatedDecl = createInstFn(ctx, originalFun, nonBoundParams);
        if ( !instantiatedDecl )
            REP_INTERNAL(loc, "Cannot instantiate generic");
        instantiatedDecl->computeType();
        theCompiler().queueSemanticCheck(instantiatedDecl->node());
        inst->setInstantiatedDecl(instantiatedDecl);

    }

    // Now actually create the call object
    DynNodeVector nonBoundArgs = getNonBoundArgs(args, genericParams());
    DynNode* res = createCallFn(loc, context, instantiatedDecl, nonBoundArgs);
    if ( !res )
        REP_INTERNAL(loc, "Cannot create code that calls generic");
    return res;
}
