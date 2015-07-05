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
    NodeVector getBoundValues(CompilationContext* context, const NodeVector& args, const NodeVector& genericParams)
    {
        NodeVector boundValues;
        boundValues.resize(args.size(), nullptr);

        // There are two types of bound arguments:
        //  - parameters with CT types
        //  - auto parameters - in this case we retain the type of the argument as a bound value
        for ( size_t i=0; i<args.size(); ++i )
        {
            Node* arg = args[i];
            Node* param = genericParams[i];
            if ( !param )
                continue;

            bool isRefAuto;
            if ( isConceptType(param->type(), isRefAuto) )
            {
                // Create a CtValue with the type of the argument corresponding to the auto parameter
                arg->computeType();
                Type* t = getAutoType(arg, isRefAuto);
                Node* typeNode = createTypeNode(context, param->location(), t);
                typeNode->computeType();
                boundValues[i] = typeNode;
            }
            else
            {
                // Evaluate the node and add the resulting CtValue as a bound argument
                arg->computeType();
                if ( !Feather::isCt(arg) )
                    return {};     // This argument must be CT in order to instantiate the generic
                Node* n = theCompiler().ctEval(arg);
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
    NodeVector getNonBoundParameters(Instantiation& inst, SprFunction* originalFun, const NodeVector& params, const NodeVector& genericParams)
    {
        const auto& boundValues = inst.boundValues();
        ASSERT(!boundValues.empty());
        NodeVector nonBoundParams;
        nonBoundParams.reserve(params.size());
        for ( size_t i=0; i<params.size(); ++i )
        {
            if ( i==0 && originalFun->hasThisParameters() )
                continue;

            Node* p = params[i];
            Node* boundValue = boundValues[i];

            if ( !genericParams[i] )            // If this is not a generic parameter => non-bound parameter
            {
                nonBoundParams.push_back(p->clone());
            }
            else if ( isConceptType(p->type()) )   // For auto-type parameters, we also create a non-bound parameter
            {
                nonBoundParams.push_back(mkSprParameter(p->location(), getName(p), boundValue));
            }
        }
        return nonBoundParams;
    }


    /// Get the eval mode for the resulting function; check the eval mode of the original function, of the non-bound arguments, and of the types
    EvalMode getResultingEvalMode(const Location& loc, EvalMode mainEvalMode, const NodeVector& args, const NodeVector& genericParams)
    {
        bool hasRtOnlyArgs = false;
        bool hasCtOnlyArgs = false;
        ASSERT(args.size() == genericParams.size());
        for ( size_t i=0; i<genericParams.size(); ++i)
        {
            // Test auto and non-bound arguments
            // Also test the type given to the 'Type' parameters (i.e., we need to know if Vector(t) can be rtct based on the mode of t)
            Type* pType = genericParams[i] ? genericParams[i]->type() : nullptr;
            Type* typeToCheck = nullptr;
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
                if ( !typeToCheck->canBeUsedAtCt() )
                    hasRtOnlyArgs = true;
                else if ( !typeToCheck->canBeUsedAtRt() )
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
    NodeVector getNonBoundArgs(const NodeVector& args, const NodeVector& genericParams)
    {
        NodeVector nonBoundArgs;
        nonBoundArgs.reserve(args.size());
        for ( size_t i=0; i<args.size(); ++i )
        {
            Node* param = genericParams[i];
            if ( !param || isConceptType(param->type()) )     // Get non-generic and also parameters
            {
                nonBoundArgs.push_back(args[i]);
            }
        }
        return nonBoundArgs;
    }

    Node* createInstFn(CompilationContext* context, SprFunction* origFun, const NodeVector& nonBoundParams)
    {
        const Location& loc = origFun->location();

        //REP_INFO(loc, "Instantiating %1% with %2% params") % getName(origFun) % nonBoundParams.size();

        NodeList* parameters = mkNodeList(loc, nonBoundParams);
        Node* returnType = origFun->returnType();
        Node* body = origFun->body();
        returnType = returnType ? returnType->clone() : nullptr;
        body = body ? body->clone() : nullptr;
        Node* newFun = mkSprFunction(loc, getName(origFun), parameters, returnType, body);
        copyModifiersSetMode(origFun, newFun, context->evalMode());
        setShouldAddToSymTab(newFun, false);
        newFun->setContext(context);

        //REP_INFO(loc, "Instantiated %1%") % newFun->toString();
        return newFun;
    }

    Node* createCallFn(const Location& loc, CompilationContext* context, Node* inst, const NodeVector& nonBoundArgs)
    {
        SprFunction* sprFun = static_cast<SprFunction*>(inst);
        sprFun->computeType();
        if ( !sprFun->resultingFun() )
            REP_ERROR(loc, "Cannot instantiate function generic %1%") % getName(inst);
        return createFunctionCall(loc, context, sprFun->resultingFun(), nonBoundArgs);
    }
}


GenericFunction::GenericFunction(SprFunction* originalFun, NodeVector params, NodeVector genericParams, Node* ifClause)
    : Generic(originalFun, move(genericParams), ifClause, publicAccess)
{
    setEvalMode(this, effectiveEvalMode(originalFun));
    referredNodes_.push_back(mkNodeList(location_, move(params)));
}

GenericFunction::~GenericFunction()
{
}

const NodeVector& GenericFunction::params() const
{
    ASSERT(referredNodes_.size() == 2);
    return referredNodes_[1]->children();
}


GenericFunction* GenericFunction::createGeneric(SprFunction* originalFun, NodeList* parameters, Node* ifClause, Class* thisClass)
{
    // If we are in a CT function, don't consider CT parameters
    bool inCtFun = effectiveEvalMode(originalFun) == modeCt;
    // For CT-generics, we consider all the parameters to be generic parameters
    bool isCtGeneric = originalFun->hasProperty(propCtGeneric);

    // Check if we have some CT parameters
    ASSERT(parameters);
    const NodeVector& params = parameters->children();
    NodeVector ourParams(params.size(), nullptr);
    NodeVector genericParams(params.size(), nullptr);
    bool hasGenericParams = false;
    for ( size_t i=0; i<params.size(); ++i )
    {
        Node* param = params[i];
        param->computeType();
        ASSERT(param->type());

        ourParams[i] = param;
        if ( isConceptType(param->type()) )
        {
            genericParams[i] = param;
            hasGenericParams = true;
        }
        if ( (!inCtFun || isCtGeneric) && isCt(param) )
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
        Type* thisType = Type::fromBasicType(getDataType(thisClass, 1, effectiveEvalMode(originalFun)));
        Node* thisParam = mkSprParameter(originalFun->location(), "$this", thisType);
        thisParam->setContext(originalFun->childrenContext());
        thisParam->computeType();
        ourParams.insert(ourParams.begin(), thisParam);
        genericParams.insert(genericParams.begin(), nullptr);
    }

    // Actually create the generic
    GenericFunction* res = new GenericFunction(originalFun, move(ourParams), move(genericParams), ifClause);
    setEvalMode(res, effectiveEvalMode(originalFun));
    res->setContext(originalFun->context());
    return res;
}

size_t GenericFunction::paramsCount() const
{
    return params().size();
}

Node* GenericFunction::param(size_t idx) const
{
    return params()[idx];
}

Instantiation* GenericFunction::canInstantiate(const NodeVector& args)
{
    Node* originalFun = referredNodes_[0];
    NodeVector boundValues = getBoundValues(originalFun->context(), args, genericParams());

    EvalMode resultingEvalMode = originalFun->hasProperty(propCtGeneric)
        ? modeCt        // If we have a CT generic, the resulting eval mode is always CT
        : getResultingEvalMode(originalFun->location(), effectiveEvalMode(originalFun), args, genericParams());

    InstantiationsSet* instantiationsSet = children_[0]->as<InstantiationsSet>();
    return instantiationsSet->canInstantiate(boundValues, resultingEvalMode);
}

Node* GenericFunction::instantiateGeneric(const Location& loc, CompilationContext* context, const NodeVector& args, Instantiation* inst)
{
    ASSERT(inst);

    // If not already created, create the actual instantiation declaration
    Node* instantiatedDecl = inst->instantiatedDecl();
    NodeList* expandedInstantiation = inst->expandedInstantiation();
    if ( !instantiatedDecl )
    {
        SprFunction* originalFun = referredNodes_[0]->as<SprFunction>();
        NodeVector nonBoundParams = getNonBoundParameters(*inst, originalFun, params(), genericParams());

        // Create the actual instantiation declaration
        CompilationContext* ctx = expandedInstantiation->childrenContext();
        instantiatedDecl = createInstFn(ctx, originalFun, nonBoundParams);
        if ( !instantiatedDecl )
            REP_INTERNAL(loc, "Cannot instantiate generic");
        instantiatedDecl->computeType();
        theCompiler().queueSemanticCheck(instantiatedDecl);
        inst->setInstantiatedDecl(instantiatedDecl);

    }

    // Now actually create the call object
    NodeVector nonBoundArgs = getNonBoundArgs(args, genericParams());
    Node* res = createCallFn(loc, context, instantiatedDecl, nonBoundArgs);
    if ( !res )
        REP_INTERNAL(loc, "Cannot create code that calls generic");
    return res;
}
