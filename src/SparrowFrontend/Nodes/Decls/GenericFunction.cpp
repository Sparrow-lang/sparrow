#include <StdInc.h>
#include "GenericFunction.h"
#include "Instantiation.h"
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
            if ( isConceptType(param->type, isRefAuto) )
            {
                // Create a CtValue with the type of the argument corresponding to the auto parameter
                Nest::computeType(arg);
                TypeRef t = getAutoType(arg, isRefAuto);
                Node* typeNode = createTypeNode(context, param->location, t);
                Nest::computeType(typeNode);
                boundValues[i] = typeNode;
            }
            else
            {
                // Evaluate the node and add the resulting CtValue as a bound argument
                Nest::computeType(arg);
                if ( !Feather::isCt(arg) )
                    return {};     // This argument must be CT in order to instantiate the generic
                Node* n = theCompiler().ctEval(arg);
                if ( !n || n->nodeKind != nkFeatherExpCtValue )
                    REP_INTERNAL(arg->location, "Invalid argument %1% when instantiating generic") % (i+1);
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
    NodeVector getNonBoundParameters(Instantiation& inst, Node* origFun, const NodeVector& params, const NodeVector& genericParams)
    {
        const auto& boundValues = inst.boundValues();
        ASSERT(!boundValues.empty());
        NodeVector nonBoundParams;
        nonBoundParams.reserve(params.size());
        for ( size_t i=0; i<params.size(); ++i )
        {
            if ( i==0 && funHasThisParameters(origFun) )
                continue;

            Node* p = params[i];
            Node* boundValue = boundValues[i];

            if ( !genericParams[i] )            // If this is not a generic parameter => non-bound parameter
            {
                nonBoundParams.push_back(cloneNode(p));
            }
            else if ( isConceptType(p->type) )   // For auto-type parameters, we also create a non-bound parameter
            {
                nonBoundParams.push_back(mkSprParameter(p->location, getName(p), boundValue));
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
            TypeRef pType = genericParams[i] ? genericParams[i]->type : nullptr;
            TypeRef typeToCheck = nullptr;
            if ( !pType || isConceptType(pType) )
            {
                Nest::computeType(args[i]);
                typeToCheck = args[i]->type;
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
    NodeVector getNonBoundArgs(const NodeVector& args, const NodeVector& genericParams)
    {
        NodeVector nonBoundArgs;
        nonBoundArgs.reserve(args.size());
        for ( size_t i=0; i<args.size(); ++i )
        {
            Node* param = genericParams[i];
            if ( !param || isConceptType(param->type) )     // Get non-generic and also parameters
            {
                nonBoundArgs.push_back(args[i]);
            }
        }
        return nonBoundArgs;
    }

    Node* createInstFn(CompilationContext* context, Node* origFun, const NodeVector& nonBoundParams)
    {
        const Location& loc = origFun->location;

        //REP_INFO(loc, "Instantiating %1% with %2% params") % getName(origFun) % nonBoundParams.size();

        Node* parameters = mkNodeList(loc, nonBoundParams);
        Node* returnType = origFun->children[1];
        Node* body = origFun->children[2];
        returnType = returnType ? cloneNode(returnType) : nullptr;
        body = body ? cloneNode(body) : nullptr;
        Node* newFun = mkSprFunction(loc, getName(origFun), parameters, returnType, body);
        copyModifiersSetMode(origFun, newFun, context->evalMode());
        setShouldAddToSymTab(newFun, false);
        Nest::setContext(newFun, context);

        //REP_INFO(loc, "Instantiated %1%") % newFun->toString();
        return newFun;
    }

    Node* createCallFn(const Location& loc, CompilationContext* context, Node* inst, const NodeVector& nonBoundArgs)
    {
        ASSERT(inst->nodeKind == nkSparrowDeclSprFunction);
        Nest::computeType(inst);
        Node* resultingFun = explanation(inst);
        if ( !resultingFun )
            REP_ERROR(loc, "Cannot instantiate function generic %1%") % getName(inst);
        return createFunctionCall(loc, context, resultingFun, nonBoundArgs);
    }
}


GenericFunction::GenericFunction(Node* originalFun, NodeVector params, NodeVector genericParams, Node* ifClause)
    : Generic(classNodeKind(), originalFun, move(genericParams), ifClause, publicAccess)
{
    setEvalMode(node(), effectiveEvalMode(originalFun));
    data_.referredNodes.push_back(mkNodeList(data_.location, move(params)));
}

GenericFunction::~GenericFunction()
{
}

const NodeVector& GenericFunction::params() const
{
    ASSERT(data_.referredNodes.size() == 2);
    return reinterpret_cast<const NodeVector&>(data_.referredNodes[1]->children);
}


GenericFunction* GenericFunction::createGeneric(Node* originalFun, Node* parameters, Node* ifClause, Node* thisClass)
{
    // If we are in a CT function, don't consider CT parameters
    bool inCtFun = effectiveEvalMode(originalFun) == modeCt;
    // For CT-generics, we consider all the parameters to be generic parameters
    bool isCtGeneric = Nest::hasProperty(originalFun, propCtGeneric);

    // Check if we have some CT parameters
    ASSERT(parameters);
    const NodeVector& params = parameters->children;
    NodeVector ourParams(params.size(), nullptr);
    NodeVector genericParams(params.size(), nullptr);
    bool hasGenericParams = false;
    for ( size_t i=0; i<params.size(); ++i )
    {
        Node* param = params[i];
        Nest::computeType(param);
        ASSERT(param->type);

        ourParams[i] = param;
        if ( isConceptType(param->type) )
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
        TypeRef thisType = getDataType(thisClass, 1, effectiveEvalMode(originalFun));
        Node* thisParam = mkSprParameter(originalFun->location, "$this", thisType);
        Nest::setContext(thisParam, Nest::childrenContext(originalFun));
        Nest::computeType(thisParam);
        ourParams.insert(ourParams.begin(), thisParam);
        genericParams.insert(genericParams.begin(), nullptr);
    }

    // Actually create the generic
    GenericFunction* res = new GenericFunction(originalFun, move(ourParams), move(genericParams), ifClause);
    setEvalMode(res->node(), effectiveEvalMode(originalFun));
    Nest::setContext(res->node(), originalFun->context);
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
    Node* originalFun = data_.referredNodes[0];
    NodeVector boundValues = getBoundValues(originalFun->context, args, genericParams());

    EvalMode resultingEvalMode = Nest::hasProperty(originalFun, propCtGeneric)
        ? modeCt        // If we have a CT generic, the resulting eval mode is always CT
        : getResultingEvalMode(originalFun->location, effectiveEvalMode(originalFun), args, genericParams());

    InstantiationsSet* instantiationsSet = (InstantiationsSet*) data_.children[0];
    return instantiationsSet->canInstantiate(boundValues, resultingEvalMode);
}

Node* GenericFunction::instantiateGeneric(const Location& loc, CompilationContext* context, const NodeVector& args, Instantiation* inst)
{
    ASSERT(inst);

    // If not already created, create the actual instantiation declaration
    Node* instantiatedDecl = inst->instantiatedDecl();
    Node* expandedInstantiation = inst->expandedInstantiation();
    if ( !instantiatedDecl )
    {
        Node* originalFun = data_.referredNodes[0];
        ASSERT(originalFun->nodeKind == nkSparrowDeclSprFunction);
        NodeVector nonBoundParams = getNonBoundParameters(*inst, originalFun, params(), genericParams());

        // Create the actual instantiation declaration
        CompilationContext* ctx = Nest::childrenContext(expandedInstantiation);
        instantiatedDecl = createInstFn(ctx, originalFun, nonBoundParams);
        if ( !instantiatedDecl )
            REP_INTERNAL(loc, "Cannot instantiate generic");
        Nest::computeType(instantiatedDecl);
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
