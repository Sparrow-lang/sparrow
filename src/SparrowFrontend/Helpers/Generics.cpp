#include <StdInc.h>
#include "Generics.h"
#include "SprTypeTraits.h"

#include <SparrowFrontendTypes.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/CommonCode.h>
#include <Helpers/Ct.h>

#include <Feather/Util/Decl.h>
#include <Feather/Util/Context.h>

#include <Nest/Frontend/SourceCode.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace
{
    ////////////////////////////////////////////////////////////////////////////
    // Instantiation
    //
    const NodeVector& instantiationBoundValues(Node* inst)
    {
        return inst->referredNodes;
    }

    Node*& expandedInstantiation(Node* inst)
    {
        return inst->children[0];
    }

    Node* instantiatedDecl(Node* inst)
    {
        return Nest_getCheckPropertyNode(inst, "instantiatedDecl");
    }

    void setInstantiatedDecl(Node* inst, Node* decl)
    {
        Nest_setProperty(inst, "instantiatedDecl", decl);
        expandedInstantiation(inst)->children.push_back(decl);
    }

    bool instantiationIsValid(Node* inst)
    {
        return 0 != Nest_getCheckPropertyInt(inst, "instIsValid");
    }

    void setInstantiationValid(Node* inst, bool valid = true)
    {
        Nest_setProperty(inst, "instIsValid", valid ? 1 : 0);
    }

    ////////////////////////////////////////////////////////////////////////////
    // InstantiationsSet
    //

    const NodeVector& getInstantiationsSetParameters(Node* instSet)
    {
        return instSet->referredNodes[1]->children;
    }

    /// In a generic we need to be able to access the bound variables; moreover the if clause might reference them.
    /// For this, we create a set of variables corresponding to the instantiation bound values. These variables are
    /// created for each instantiated and put in the node-list of the instantiation (the expanded instantiation node).
    /// Note that for auto-parameters we will create RT variables; the only thing we can do with them is to use their type
    /// In the expanded instantiation we need to add the actual instantiated declaration - in other place, not here
    NodeVector getBoundVariables(const Location& loc, const NodeVector& boundValues, const NodeVector& params, bool insideClass)
    {
        // Create a variable for each bound parameter - put everything in a node list
        NodeVector nodes;
        NodeVector nonBoundParams;
        size_t idx = 0;
        for ( Node* p: params )
        {
            Node* boundValue = boundValues[idx++];
            if ( !p )
                continue;
            ASSERT(boundValue);

            if ( isConceptType(p->type) )
            {
                TypeRef t = getType(boundValue);

                Node* var = mkSprVariable(p->location, getName(p), t, nullptr);
                if ( insideClass )
                    Nest_setProperty(var, propIsStatic, 1);
                nodes.push_back(var);
            }
            else
            {
                Node* var = mkSprVariable(p->location, getName(p), boundValue->type, boundValue);
                if ( insideClass )
                    Nest_setProperty(var, propIsStatic, 1);
                setEvalMode(var, modeCt);
                nodes.push_back(var);
            }
        }
        nodes.push_back(mkNop(loc));    // Make sure the resulting type is Void
        return nodes;
    }

    Node* searchInstantiation(Node* instSet, const NodeVector& values)
    {
        NodeVector& instantiations = instSet->children[1]->children;
        for ( Node* inst: instantiations )
        {
            const auto& boundValues = inst->referredNodes;
            if ( boundValues.size() != values.size() )
                continue;

            bool argsMatch = true;
            for ( size_t i=0; i<values.size(); ++i )
            {
                if ( !boundValues[i] )
                    continue;
                if ( !values[i] || !ctValsEqual(values[i], boundValues[i]) )
                {
                    argsMatch = false;
                    break;
                }
            }
            if ( argsMatch )
                return inst;
        }
        return nullptr;
    }

    Node* createNewInstantiation(Node* instSet, const NodeVector& values, EvalMode evalMode)
    {
        // Create a new context, but at the same level as the context of the parent node
        Node* parentNode = instSet->referredNodes[0];
        CompilationContext* context = Nest_mkChildContextWithSymTab(parentNode->context, nullptr, evalMode);
        bool insideClass = nullptr != getParentClass(context);

        // Create the instantiation
        auto boundVars = getBoundVariables(instSet->location, values, getInstantiationsSetParameters(instSet), insideClass);
        Node* inst = mkInstantiation(instSet->location, values, move(boundVars));
        NodeVector& instantiations = instSet->children[1]->children;
        instantiations.push_back(inst);

        // Compile the newly created instantiation
        Nest_setContext(expandedInstantiation(inst), context);
        Nest_semanticCheck(expandedInstantiation(inst));

        return inst;
    }

    Node* canInstantiate(Node* instSet, const NodeVector& values, EvalMode evalMode)
    {
        // Try to find an existing instantiation
        Node* inst = searchInstantiation(instSet, values);
        if ( inst )
        {
            // We already checked whether we can instantiate this
            return instantiationIsValid(inst) ? inst : nullptr;
        }

        // If no instantiation is found, create a new instantiation
        inst = createNewInstantiation(instSet, values, evalMode);

        // If we have an if clause, check if this CT evaluates to true
        Node* ifClause = instSet->children[0];
        if ( ifClause )
        {
            // Always use a clone of the original node
            Node* cond = Nest_cloneNode(ifClause);
            Nest_setContext(cond, Nest_childrenContext(expandedInstantiation(inst)));

            // If the condition does not compile, we cannot instantiate
            bool isValid = false;
            int oldVal = Nest_isReportingEnabled();
            Nest_enableReporting(0);
            try
            {
                Nest_semanticCheck(cond);
                isValid = !cond->nodeError
                    && Feather::isCt(cond)          // We must have a value at CT
                    && Feather::isTestable(cond)    // The value must be boolean
                    && Nest_getSuppressedErrorsNum() == 0;  // No suppressed errors
            }
            catch (...)
            {
            }
            Nest_enableReporting(oldVal);
            if ( !isValid )
                return nullptr;

            // Evaluate the if clause condition and check the result
            if ( !SprFrontend::getBoolCtValue(theCompiler().ctEval(cond)) )
                return nullptr;
        }

        setInstantiationValid(inst);
        return inst;
    }



    ////////////////////////////////////////////////////////////////////////////
    // Generic class
    //

    const NodeVector& genericParams(Node* generic)
    {
        return getInstantiationsSetParameters(generic->children[0]);
    }

    /// Get the bound arguments corresponding to the arguments passed to the generic
    /// We return here the CT values of these arguments; we use their value to check for duplicate instantiations
    NodeVector getGenericClassBoundValues(const NodeVector& args)
    {
        NodeVector boundValues;
        boundValues.reserve(args.size());

        for ( size_t i=0; i<args.size(); ++i )
        {
            Node* arg = args[i];

            // Evaluate the node and add the resulting CtValue as a bound argument
            Nest_computeType(arg);
            if ( !Feather::isCt(arg) )
                REP_INTERNAL(arg->location, "Argument to a class generic must be CT (type: %1%)") % arg->type;
            Node* n = theCompiler().ctEval(arg);
            if ( !n || n->nodeKind != nkFeatherExpCtValue )
                REP_INTERNAL(arg->location, "Invalid argument %1% when instantiating generic") % (i+1);
            boundValues.push_back(n);
        }
        return boundValues;
    }

    EvalMode getGenericClassResultingEvalMode(const Location& loc, EvalMode mainEvalMode, const NodeVector& boundValues)
    {
        bool hasRtOnlyArgs = false;
        bool hasCtOnlyArgs = false;
        for ( Node* boundVal: boundValues )
        {
            // Test the type given to the 'Type' parameters (i.e., we need to know if Vector(t) can be rtct based on the mode of t)
            TypeRef t = tryGetTypeValue(boundVal);
            if ( t )
            {
                if ( t->mode == modeRt )
                    hasRtOnlyArgs = true;
                else if ( t->mode == modeCt )
                    hasCtOnlyArgs = true;
            }
            else if ( !boundVal->type->canBeUsedAtRt )
            {
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

    Node* createInstantiatedClass(CompilationContext* context, Node* orig, const string& description)
    {
        const Location& loc = orig->location;

        Node* baseClasses = orig->children[1];
        Node* children = orig->children[2];
        baseClasses = baseClasses ? Nest_cloneNode(baseClasses) : nullptr;
        children = children ? Nest_cloneNode(children) : nullptr;
        Node* newClass = mkSprClass(loc, getName(orig), nullptr, baseClasses, nullptr, children);

        copyModifiersSetMode(orig, newClass, context->evalMode);

        //setShouldAddToSymTab(newClass, false);    // TODO (generics): Uncomment this line
        Nest_setContext(newClass, context);

//        REP_INFO(loc, "Instantiated %1%") % description;
        return newClass;
    }

    string getGenericClassDescription(Node* cls, Node* inst)
    {
        ostringstream oss;
        oss << getName(cls) << "[";
        const auto& boundValues = instantiationBoundValues(inst);
        for ( size_t i=0; i<boundValues.size(); ++i )
        {
            if ( i>0 )
                oss << ", ";
            TypeRef t = evalTypeIfPossible(boundValues[i]);
            if ( t )
                oss << t;
            else
                oss << boundValues[i];
        }
        oss << "]";
        return oss.str();
    }

    ////////////////////////////////////////////////////////////////////////////
    // Generic function
    //

    /// Get the bound arguments corresponding to the CT or Auto parameters of the generic
    /// We return here the CT values of these arguments; we use their value to check for duplicate instantiations
    NodeVector getGenericFunBoundValues(CompilationContext* context, const NodeVector& args, const NodeVector& genericParams)
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
                Nest_computeType(arg);
                TypeRef t = getAutoType(arg, isRefAuto);
                Node* typeNode = createTypeNode(context, param->location, t);
                Nest_computeType(typeNode);
                boundValues[i] = typeNode;
            }
            else
            {
                // Evaluate the node and add the resulting CtValue as a bound argument
                Nest_computeType(arg);
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
    NodeVector getGenericFunNonBoundParameters(Node* inst, Node* origFun, const NodeVector& params, const NodeVector& genericParams)
    {
        const auto& boundValues = instantiationBoundValues(inst);
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
                nonBoundParams.push_back(Nest_cloneNode(p));
            }
            else if ( isConceptType(p->type) )   // For auto-type parameters, we also create a non-bound parameter
            {
                nonBoundParams.push_back(mkSprParameter(p->location, getName(p), boundValue));
            }
        }
        return nonBoundParams;
    }


    /// Get the eval mode for the resulting function; check the eval mode of the original function, of the non-bound arguments, and of the types
    EvalMode getGenericFunResultingEvalMode(const Location& loc, EvalMode mainEvalMode, const NodeVector& args, const NodeVector& genericParams)
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
                Nest_computeType(args[i]);
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
    NodeVector getGenericFunNonBoundArgs(const NodeVector& args, const NodeVector& genericParams)
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
        returnType = returnType ? Nest_cloneNode(returnType) : nullptr;
        body = body ? Nest_cloneNode(body) : nullptr;
        Node* newFun = mkSprFunction(loc, getName(origFun), parameters, returnType, body);
        copyModifiersSetMode(origFun, newFun, context->evalMode);
        setShouldAddToSymTab(newFun, false);
        Nest_setContext(newFun, context);

        //REP_INFO(loc, "Instantiated %1%") % newFun->toString();
        return newFun;
    }

    Node* createCallFn(const Location& loc, CompilationContext* context, Node* inst, const NodeVector& nonBoundArgs)
    {
        ASSERT(inst->nodeKind == nkSparrowDeclSprFunction);
        Nest_computeType(inst);
        Node* resultingFun = Nest_explanation(inst);
        if ( !resultingFun )
            REP_ERROR(loc, "Cannot instantiate function generic %1%") % getName(inst);
        return createFunctionCall(loc, context, resultingFun, nonBoundArgs);
    }
}

bool SprFrontend::conceptIsFulfilled(Node* concept, TypeRef type)
{
    Node* instantiationsSet = concept->children[2];

    if ( !concept->nodeSemanticallyChecked || !instantiationsSet )
        REP_INTERNAL(concept->location, "Invalid concept");

    Node* typeValue = createTypeNode(concept->context, concept->location, type);
    Nest_semanticCheck(typeValue);

    return nullptr != canInstantiate(instantiationsSet, {typeValue}, concept->context->evalMode);
}

TypeRef SprFrontend::baseConceptType(Node* concept)
{
    Node* baseConcept = concept->children[0];

    TypeRef res = baseConcept ? getType(baseConcept) : getConceptType();
    res = adjustMode(res, concept->context, concept->location);
    return res;
}

Node* SprFrontend::createGenericFun(Node* originalFun, Node* parameters, Node* ifClause, Node* thisClass)
{
    // If we are in a CT function, don't consider CT parameters
    bool inCtFun = effectiveEvalMode(originalFun) == modeCt;
    // For CT-generics, we consider all the parameters to be generic parameters
    bool isCtGeneric = Nest_hasProperty(originalFun, propCtGeneric);

    // Check if we have some CT parameters
    ASSERT(parameters);
    const NodeVector& params = parameters->children;
    NodeVector ourParams(params.size(), nullptr);
    NodeVector genericParams(params.size(), nullptr);
    bool hasGenericParams = false;
    for ( size_t i=0; i<params.size(); ++i )
    {
        Node* param = params[i];
        Nest_computeType(param);
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
        Nest_setContext(thisParam, Nest_childrenContext(originalFun));
        Nest_computeType(thisParam);
        ourParams.insert(ourParams.begin(), thisParam);
        genericParams.insert(genericParams.begin(), nullptr);
    }

    // Actually create the generic
    Node* res = mkGenericFunction(originalFun, move(ourParams), move(genericParams), ifClause);
    setEvalMode(res, effectiveEvalMode(originalFun));
    Nest_setContext(res, originalFun->context);
    return res;
}

bool SprFrontend::isGeneric(const Node* node)
{
    return node->nodeKind == nkSparrowDeclGenericClass
        || node->nodeKind == nkSparrowDeclGenericFunction;
}

size_t SprFrontend::genericParamsCount(const Node* node)
{
    switch ( node->nodeKind - firstSparrowNodeKind )
    {
        case nkRelSparrowDeclGenericClass:
        {
            Node* instantiationsSet = node->children[0];
            return getInstantiationsSetParameters(instantiationsSet).size();
        }
        case nkRelSparrowDeclGenericFunction:
            return node->referredNodes[1]->children.size();
        default:
            REP_INTERNAL(node->location, "Node is not a generic: %1%") % node;
            return 0;
    }
}
Node* SprFrontend::genericParam(const Node* node, size_t idx)
{
    switch ( node->nodeKind - firstSparrowNodeKind )
    {
        case nkRelSparrowDeclGenericClass:
        {
            Node* instantiationsSet = node->children[0];
            return getInstantiationsSetParameters(instantiationsSet)[idx];
        }
        case nkRelSparrowDeclGenericFunction:
            return node->referredNodes[1]->children[idx];
        default:
            REP_INTERNAL(node->location, "Node is not a generic: %1%") % node;
            return nullptr;
    }
}

Node* SprFrontend::genericCanInstantiate(Node* node, const NodeVector& args)
{
    switch ( node->nodeKind - firstSparrowNodeKind )
    {
        case nkRelSparrowDeclGenericClass:
        {
            NodeVector boundValues = getGenericClassBoundValues(args);
            Node* originalClass = node->referredNodes[0];
            EvalMode resultingEvalMode = getGenericClassResultingEvalMode(originalClass->location, effectiveEvalMode(originalClass), boundValues);
            Node* instantiationsSet = node->children[0];
            return canInstantiate(instantiationsSet, boundValues, resultingEvalMode);
        }
        case nkRelSparrowDeclGenericFunction:
        {
            Node* originalFun = node->referredNodes[0];
            NodeVector boundValues = getGenericFunBoundValues(originalFun->context, args, genericParams(node));

            EvalMode resultingEvalMode = Nest_hasProperty(originalFun, propCtGeneric)
                ? modeCt        // If we have a CT generic, the resulting eval mode is always CT
                : getGenericFunResultingEvalMode(originalFun->location, effectiveEvalMode(originalFun), args, genericParams(node));

            Node* instantiationsSet = node->children[0];
            return canInstantiate(instantiationsSet, boundValues, resultingEvalMode);
        }
        default:
            REP_INTERNAL(node->location, "Node is not a generic: %1%") % node;
            return nullptr;
    }
}
Node* SprFrontend::genericDoInstantiate(Node* node, const Location& loc, CompilationContext* context, const NodeVector& args, Node* inst)
{
    switch ( node->nodeKind - firstSparrowNodeKind )
    {
        case nkRelSparrowDeclGenericClass:
        {
            ASSERT(inst);

            // If not already created, create the actual instantiation declaration
            Node* instDecl = instantiatedDecl(inst);
            Node* expandedInst = expandedInstantiation(inst);
            if ( !instDecl )
            {
                Node* originalClass = Nest_ofKind(node->referredNodes[0], nkSparrowDeclSprClass);
                string description = getGenericClassDescription(originalClass, inst);

                // Create the actual instantiation declaration
                CompilationContext* ctx = Nest_childrenContext(expandedInst);
                instDecl = createInstantiatedClass(ctx, originalClass, description);
                if ( !instDecl )
                    REP_INTERNAL(loc, "Cannot instantiate generic");
                Nest_setProperty(instDecl, propDescription, move(description));
                Nest_computeType(instDecl);
                theCompiler().queueSemanticCheck(instDecl);
                setInstantiatedDecl(inst, instDecl);

                // Add the instantiated class as an additional node to the callee source code
                ASSERT(context->sourceCode);
                context->sourceCode->additionalNodes.push_back(expandedInst);
            }

            // Now actually create the call object: a Type CT value
            Node* cls = Nest_ofKind(Nest_explanation(instDecl), nkFeatherDeclClass);
            ASSERT(cls);
            return createTypeNode(node->context, loc, Feather::getDataType(cls));
        }
        case nkRelSparrowDeclGenericFunction:
        {
            ASSERT(inst);

            // If not already created, create the actual instantiation declaration
            Node* instDecl = instantiatedDecl(inst);
            Node* expandedInst = expandedInstantiation(inst);
            if ( !instDecl )
            {
                Node* originalFun = node->referredNodes[0];
                ASSERT(originalFun->nodeKind == nkSparrowDeclSprFunction);
                NodeVector nonBoundParams = getGenericFunNonBoundParameters(inst, originalFun, node->referredNodes[1]->children, genericParams(node));

                // Create the actual instantiation declaration
                CompilationContext* ctx = Nest_childrenContext(expandedInst);
                instDecl = createInstFn(ctx, originalFun, nonBoundParams);
                if ( !instDecl )
                    REP_INTERNAL(loc, "Cannot instantiate generic");
                Nest_computeType(instDecl);
                theCompiler().queueSemanticCheck(instDecl);
                setInstantiatedDecl(inst, instDecl);

            }

            // Now actually create the call object
            NodeVector nonBoundArgs = getGenericFunNonBoundArgs(args, genericParams(node));
            Node* res = createCallFn(loc, context, instDecl, nonBoundArgs);
            if ( !res )
                REP_INTERNAL(loc, "Cannot create code that calls generic");
            return res;
        }
        default:
            REP_INTERNAL(node->location, "Node is not a generic: %1%") % node;
            return nullptr;
    }
}
