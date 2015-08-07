#include <StdInc.h>
#include "CommonCode.h"
#include "DeclsHelpers.h"
#include "Overload.h"
#include "Convert.h"
#include "Ct.h"
#include "StdDef.h"
#include "SprTypeTraits.h"
#include <NodeCommonsCpp.h>
#include <Nodes/Decls/Generic.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>

#include <Nest/CompilerSettings.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

Node* SprFrontend::createCtorCall(const Location& loc, CompilationContext* context, NodeVector args)
{
    if ( args.empty() )
        REP_INTERNAL(loc, "At least 'this' argument must be given when creating a ctor call");

    // Get the class from 'thisArg'
    Node* thisArg = args[0];
    computeType(thisArg);
    Node* cls = classForType(thisArg->type);
    CHECK(loc, cls);

    // Check if we can apply RVO, or pseudo-RVO
    // Whenever we try to construct an object from another temporary object, try to bypass the temporary object creation
    if ( args.size() == 2 && !theCompiler().settings().noRVO_ && !isCt(thisArg) )
    {
        Node* arg = args[1];
        computeType(arg);
        arg = explanation(arg);
        if ( classForType(arg->type) == cls )
        {
            Node*const* tempVarConstruction1 = getPropertyNode(arg, propTempVarContstruction);
            Node* tempVarConstruction = tempVarConstruction1 ? *tempVarConstruction1 : nullptr;
            if ( tempVarConstruction && tempVarConstruction->nodeKind == nkFeatherExpFunCall )
            {
                Node* fnCall = tempVarConstruction;
                ASSERT(fnCall->children.size() >= 1);

                // This argument - make sure it's of the required type
                Node* thisParam = Function_getParameter(fnCall->referredNodes[0], 0);
                TypeRef thisParamType = thisParam->type;
                ConversionResult cvt = canConvert(thisArg, thisParamType);
                if ( !cvt )
                    REP_INTERNAL(loc, "Cannot convert this arg in RVO (%1% -> %2%)") % thisArg->type % thisParamType;
                Node* thisArg1 = cvt.apply(thisArg);

                // Create a new call based on the original temp var construction call, but changing the this argument
                NodeVector args;
                args.reserve(fnCall->children.size()+1);
                args.push_back(thisArg1);
                for ( size_t i=1; i<fnCall->children.size(); ++i )
                    args.push_back(fnCall->children[i]);
                Node* newCall = mkFunCall(loc, fnCall->referredNodes[0], move(args));
                setContext(newCall, context);
                return newCall;
            }
        }
    }

    // Search for the ctors in the class
    NodeVector decls = cls->childrenContext->currentSymTab()->lookupCurrent("ctor");

    // If no declarations found, just don't initialize the object
    if ( decls.empty() )
        return nullptr;

    // Do the overloading procedure to select the right ctor
    return selectOverload(context, loc, thisArg->type->mode, move(decls), args, true, "ctor");
}

Node* SprFrontend::createCtorCall(const Location& loc, CompilationContext* context, Node* thisArg, Node* initArg)
{
    NodeVector args;
    args.push_back(thisArg);
    if ( initArg )
        args.push_back(initArg);
    return createCtorCall(loc, context, args);
}

Node* SprFrontend::createDtorCall(const Location& loc, CompilationContext* context, Node* thisArg)
{
    // Get the class from 'thisArg'
    computeType(thisArg);
    Node* cls = classForType(thisArg->type);
    CHECK(loc, cls);

    // Search for the dtor in the class 
    NodeVector decls = cls->childrenContext->currentSymTab()->lookupCurrent("dtor");

    // If no destructor found, don't call anything
    if ( decls.empty() )
        return nullptr;

    // Sanity checks
    if ( decls.size() > 1 )
        REP_ERROR(loc, "Multiple destructors found for class %1%") % getName(cls);
    Node* dtor = explanation(decls.front());
    if ( !dtor || dtor->nodeKind != nkFeatherDeclFunction )
        REP_ERROR(decls.front()->location, "Invalid destructor found for class %1%") % getName(cls);
    if ( Function_numParameters(dtor) != 1 )
        REP_INTERNAL(decls.front()->location, "Invalid destructor found for class %1%; it has %2% parameters") % getName(cls) % Function_numParameters(dtor);

    // Check this parameter
    TypeRef thisParamType = Function_getParameter(dtor, 0)->type;
    if ( Feather::isCt(thisArg) )
        thisParamType = Feather::changeTypeMode(thisParamType, modeCt, thisArg->location);
    ConversionResult c = canConvert(thisArg, thisParamType);
    if ( !c )
        REP_INTERNAL(loc, "Invalid this argument when calling dtor");
    Node* argWithConversion = c.apply(thisArg);

    Node* funCall = mkFunCall(loc, dtor, NodeVector(1, argWithConversion));
    setContext(funCall, context);
    return funCall;
}

Node* SprFrontend::createFunctionCall(const Location& loc, CompilationContext* context, Node* fun, NodeVector args)
{
    ASSERT(context);
    computeType(fun);

    // Set the arguments to the function call.
    // If we have a result param, create a temporary variable for it, and call the function with it; then we return the
    // content of the variable
    Node* tmpVarRef = nullptr;
    Node* res = nullptr;
    Node* resultParam = getResultParam(fun);
    if ( resultParam )
    {
        // Get the resulting type; check for CT-ness
        TypeRef resTypeRef = resultParam->type;
        EvalMode funEvalMode = effectiveEvalMode(fun);
        if ( funEvalMode == modeCt && !isCt(resTypeRef) )
            resTypeRef = changeTypeMode(resTypeRef, modeCt, resultParam->location);
        if ( funEvalMode == modeRtCt && hasProperty(fun, propAutoCt) && !isCt(resTypeRef) && isCt(args) )
            resTypeRef = changeTypeMode(resTypeRef, modeCt, resultParam->location);

        // Create a temporary variable for the result
        Node* tmpVar = Feather::mkVar(loc, "$tmpC", mkTypeNode(loc, removeRef(resTypeRef)));
        setContext(tmpVar, context);
        tmpVarRef = mkVarRef(loc, tmpVar);
        setContext(tmpVarRef, context);

        // Add a new argument with the temporary variable
        NodeVector args1 = args;
        Node* arg = mkBitcast(tmpVarRef->location, mkTypeNode(loc, resTypeRef), tmpVarRef);
        setContext(arg, context);
        args1.insert(args1.begin(), arg);
        Node* funCall = mkFunCall(loc, fun, args1);

        res = createTempVarConstruct(loc, context, funCall, tmpVar);

        // TODO: Check why we cannot return a reference when the result is a type
        if ( resTypeRef == StdDef::typeRefType )
            res = mkMemLoad(loc, res);
    }
    else
    {
        Node* funCall = mkFunCall(loc, fun, args);
        res = funCall;
    }

    setContext(res, context);
    computeType(res);

    // CT sanity check
    //checkEvalMode(funCall, effectiveEvalMode(fun));

    return res;
}

Node* SprFrontend::createTempVarConstruct(const Location& loc, CompilationContext* context, Node* constructAction, Node* var)
{
    CHECK(loc, constructAction->nodeKind == nkFeatherExpFunCall);
    Node* funCall = constructAction;
    CHECK(loc, !funCall->children.empty());
    Node* thisArg = funCall->children[0];
    computeType(thisArg);

    // Create a temp destruct action with the call of the destructor
    Node* destructAction = nullptr;
    if ( !isCt(thisArg) )
    {
        Node* dtorCall = createDtorCall(loc, context, thisArg);
        if ( dtorCall )
            destructAction = mkTempDestructAction(loc, dtorCall);
    }

    // The result of the expressions
    Node* result = mkVarRef(loc, var);   // Return a var-ref to the temporary object

    Node* res = mkNodeList(loc, { var, constructAction, destructAction, result });
    setContext(res, context);
    computeType(res);
    setProperty(res, propTempVarContstruction, constructAction);

    // CT sanity checks
    checkEvalMode(res, var->type->mode);

    return res;
}

Node* SprFrontend::createFunPtr(Node* funNode)
{
    CompilationContext* ctx = funNode->context;
    const Location& loc = funNode->location;

    // Allow the funNode to return DeclExp
    setProperty(funNode, propAllowDeclExp, 1, true);

    Node* baseExp = nullptr;
    NodeVector decls = getDeclsFromNode(funNode, baseExp);

    // Make sure we refer only to one decl
    if ( decls.size() == 0 )
        REP_ERROR(loc, "No function found: %1%") % funNode;


    // Basic case: is this a plain function?
    Node* resDecl = decls.size() >= 1 ? resultingDecl(decls[0]) : nullptr;
    Node* fun = (resDecl && resDecl->nodeKind == nkFeatherDeclFunction) ? resDecl : nullptr;
    if ( fun )
    {
        // Does this have a result parameter?
        Node* resParam = getResultParam(resDecl);

        // Try to instantiate the corresponding FunctionPtr class
        NodeVector parameters;
        parameters.reserve(1+Function_numParameters(fun));
        TypeRef resType = resParam ? removeRef(resParam->type) : Function_resultType(fun);
        parameters.push_back(createTypeNode(ctx, loc, resType));
        for ( size_t i = resParam ? 1 : 0; i<Function_numParameters(fun); ++i )
        {
            parameters.push_back(createTypeNode(ctx, loc, Function_getParameter(fun, i)->type));
        }
        string className = "FunctionPtr";
        Node* classCall = mkFunApplication(loc, mkIdentifier(loc, className), mkNodeList(loc, parameters));
        setContext(classCall, ctx);
        computeType(classCall);

        // Get the actual class object from the instantiation
        TypeRef t = getType(classCall);

        // If the class is valid, we have a conversion
        if ( t )
            return mkFunRef(loc, fun, mkTypeNode(loc, t));
        
        REP_ERROR(loc, "Invalid function: %1%") % funNode;
        return nullptr;
    }

    // TODO: In general we should create an object that is able to call any type of callable

    // If we have a generic, try to wrap it in a lambda
    if ( isGeneric(resDecl) )
    {
        size_t numParams = genericParamsCount(resDecl);

        Node* paramsType = mkIdentifier(loc, "AnyType");

        NodeVector paramIds(numParams, nullptr);
        NodeVector args(numParams, nullptr);
        for ( size_t i=0; i<numParams; ++i )
        {
            string name = "p" + boost::lexical_cast<string>(i);
            paramIds[i] = mkSprParameter(loc, name, paramsType, nullptr);
            args[i] = mkIdentifier(loc, name);
        }

        Node* parameters = mkNodeList(loc, paramIds);
        Node* bodyExp = mkFunApplication(loc, funNode, mkNodeList(loc, args));

        Node* res = mkLambdaExp(loc, parameters, nullptr, nullptr, bodyExp, nullptr);
        return res;
    }

    if ( !fun )
        REP_ERROR(funNode->location, "The given node does not refer to a function (we have: %1%)") % decls[0];
    return nullptr;

}
