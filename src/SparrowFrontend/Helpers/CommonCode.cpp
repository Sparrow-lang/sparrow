#include <StdInc.h>
#include "CommonCode.h"
#include "DeclsHelpers.h"
#include "Overload.h"
#include "Convert.h"
#include "Ct.h"
#include "StdDef.h"
#include "SprTypeTraits.h"
#include "Generics.h"
#include "SprDebug.h"
#include <NodeCommonsCpp.h>

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

#include "Nest/Utils/CompilerSettings.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

Node* SprFrontend::createCtorCall(
        const Location& loc, CompilationContext* context, Nest_NodeRange args) {
    auto numArgs = Nest_nodeRangeSize(args);
    if (numArgs == 0)
        REP_INTERNAL(loc, "At least 'this' argument must be given when creating a ctor call");

    // Get the class from 'thisArg'
    Node* thisArg = at(args, 0);
    if (!Nest_computeType(thisArg))
        return nullptr;
    CHECK(loc, thisArg->type->hasStorage);
    Node* cls = thisArg->type->referredNode;
    CHECK(loc, cls);

    // TODO (ctors): We need to apply this even if we are calling ctor by hand
    // Example: this.subObj ctor MyType(a,b,c)
    // should be transformed into: ctor(this.subObj, a,b,c)

    // Check if we can apply RVO, or pseudo-RVO
    // Whenever we try to construct an object from another temporary object, try to bypass the
    // temporary object creation
    if (numArgs == 2 && !Nest_compilerSettings()->noRVO_ && !Feather_isCt(thisArg)) {
        Node* arg = at(args, 1);
        if (!Nest_computeType(arg))
            return nullptr;
        arg = Nest_explanation(arg);
        if (arg->type->referredNode == cls) {
            Node* const* tempVarConstruction1 = Nest_getPropertyNode(arg, propTempVarContstruction);
            Node* tempVarConstruction = tempVarConstruction1 ? *tempVarConstruction1 : nullptr;
            if (tempVarConstruction && tempVarConstruction->nodeKind == nkFeatherExpFunCall) {
                Node* fnCall = tempVarConstruction;
                ASSERT(Nest_nodeArraySize(fnCall->children) >= 1);
                Node* fun = at(fnCall->referredNodes, 0);

                // This argument - make sure it's of the required type
                Node* thisParam = FunctionDecl(fun).parameters()[0];
                Type thisParamType = thisParam->type;
                ConversionResult cvt = g_ConvertService->checkConversion(thisArg, thisParamType);
                if (!cvt)
                    REP_INTERNAL(loc, "Cannot convert this arg in RVO (%1% -> %2%)") %
                            thisArg->type % thisParamType;
                Node* thisArg1 = cvt.apply(thisArg);

                // Create a new call based on the original temp var construction call, but changing
                // the this argument
                NodeVector args;
                size_t size = Nest_nodeArraySize(fnCall->children);
                args.reserve(size);
                args.push_back(thisArg1);
                Nest_NodeRange r = Nest_nodeChildren(fnCall);
                r.beginPtr++;
                for (Node* child : r)
                    args.push_back(child);
                Node* newCall = Feather_mkFunCall(loc, fun, all(args));
                Nest_setContext(newCall, context);
                return newCall;
            }
        }
    }

    // Search for the ctors associated with the class
    auto decls = getClassAssociatedDecls(cls, "ctor");

    // If no declarations found, just don't initialize the object
    if (Nest_nodeArraySize(decls) == 0)
        return nullptr;

    // Do the overloading procedure to select the right ctor
    Node* res = g_OverloadService->selectOverload(context, loc, thisArg->type->mode, all(decls),
            args, OverloadReporting::full, StringRef("ctor"));
    Nest_freeNodeArray(decls);
    return res;
}

Node* SprFrontend::createCtorCall(
        const Location& loc, CompilationContext* context, Node* thisArg, Node* initArg) {
    NodeVector args;
    args.push_back(thisArg);
    if (initArg)
        args.push_back(initArg);
    return createCtorCall(loc, context, all(args));
}

Node* SprFrontend::createDtorCall(const Location& loc, CompilationContext* context, Node* thisArg) {
    ASSERT(thisArg);

    // Get the class from 'thisArg'
    if (!Nest_computeType(thisArg))
        return nullptr;
    CHECK(loc, thisArg->type->hasStorage);
    Node* cls = thisArg->type->referredNode;
    CHECK(loc, cls);

    // Search for the dtor associated with the class
    auto decls = getClassAssociatedDecls(cls, "dtor");

    // If no destructor found, don't call anything
    auto numDecls = Nest_nodeArraySize(decls);
    if (numDecls == 0)
        return nullptr;

    // Do the overloading procedure to select the right dtor
    // Don't report errors; having no matching dtor is a valid case
    Node* res = g_OverloadService->selectOverload(context, loc, thisArg->type->mode, all(decls),
            fromIniList({thisArg}), OverloadReporting::none, StringRef("dtor"));
    Nest_freeNodeArray(decls);
    return res; // can be null
}

bool _areNodesCt(Nest_NodeRange nodes) {
    for (Node* n : nodes) {
        if (!n->type)
            Nest_computeType(n);
        if (n->type->mode != modeCt)
            return false;
    }
    return true;
}

Node* SprFrontend::createFunctionCall(
        const Location& loc, CompilationContext* context, Node* fun, Nest_NodeRange args) {
    ASSERT(context);
    if (!Nest_computeType(fun))
        return nullptr;

    // Set the arguments to the function call.
    // If we have a result param, create a temporary variable for it, and call the function with it;
    // then we return the
    // content of the variable
    Node* tmpVarRef = nullptr;
    Node* res = nullptr;
    Node* resultParam = getResultParam(fun);
    if (resultParam) {
        // Get the resulting type; check for CT-ness
        TypeWithStorage resType = resultParam->type;
        EvalMode funEvalMode = Feather_effectiveEvalMode(fun);
        if (funEvalMode == modeCt && resType.mode() != modeCt)
            resType = resType.changeMode(modeCt, resultParam->location);
        if (funEvalMode == modeRt && Nest_hasProperty(fun, propAutoCt) &&
                resType.mode() != modeCt && _areNodesCt(args))
            resType = resType.changeMode(modeCt, resultParam->location);

        // Create a temporary variable for the result
        Node* tmpVar = Feather_mkVar(
                loc, StringRef("$tmpC"), Feather_mkTypeNode(loc, removeCatOrRef(resType)));
        Nest_setContext(tmpVar, context);
        tmpVarRef = Feather_mkVarRef(loc, tmpVar);
        Nest_setContext(tmpVarRef, context);

        // Add a new argument with the temporary variable
        NodeVector args1 = toVec(args);
        Node* arg =
                Feather_mkBitcast(tmpVarRef->location, Feather_mkTypeNode(loc, resType), tmpVarRef);
        Nest_setContext(arg, context);
        args1.insert(args1.begin(), arg);
        Node* funCall = Feather_mkFunCall(loc, fun, all(args1));

        res = createTempVarConstruct(loc, context, funCall, tmpVar);

        // TODO: Check why we cannot return a reference when the result is a type
        if (resType == StdDef::typeRefType)
            res = Feather_mkMemLoad(loc, res);
    } else {
        Node* funCall = Feather_mkFunCall(loc, fun, args);
        res = funCall;
    }

    Nest_setContext(res, context);
    if (!Nest_computeType(res))
        return nullptr;

    // CT sanity check
    Feather_checkEvalModeWithExpected(res, Feather_effectiveEvalMode(fun));

    return res;
}

Node* SprFrontend::createTempVarConstruct(
        const Location& loc, CompilationContext* context, Node* constructAction, Node* var) {
    CHECK(loc, constructAction->nodeKind == nkFeatherExpFunCall);
    Node* funCall = constructAction;
    CHECK(loc, Nest_nodeArraySize(funCall->children) != 0);
    Node* thisArg = at(funCall->children, 0);
    if (!Nest_computeType(thisArg))
        return nullptr;

    // Create a temp destruct action with the call of the destructor
    Node* destructAction = nullptr;
    if (!Feather_isCt(thisArg)) {
        Node* dtorCall = createDtorCall(loc, context, thisArg);
        if (dtorCall)
            destructAction = Feather_mkTempDestructAction(loc, dtorCall);
    }

    // The result of the expressions
    Node* result = Feather_mkVarRef(loc, var); // Return a var-ref to the temporary object

    Node* res =
            Feather_mkNodeList(loc, fromIniList({var, constructAction, destructAction, result}));
    Nest_setContext(res, context);
    if (!Nest_computeType(res))
        return nullptr;
    Nest_setPropertyNode(res, propTempVarContstruction, constructAction);

    // CT sanity checks
    Feather_checkEvalModeWithExpected(res, var->type->mode);

    return res;
}

Node* _createFunPtrForFeatherFun(Node* fun, Node* callNode) {
    ASSERT(fun);
    ASSERT(fun->nodeKind == nkFeatherDeclFunction);
    CHECK(fun->location, fun->nodeKind == nkFeatherDeclFunction);

    CompilationContext* ctx = callNode->context;
    const Location& loc = callNode->location;

    // Does this have a result parameter?
    Node* resParam = getResultParam(fun);

    // Try to instantiate the corresponding FunctionPtr class
    NodeVector parameters;
    parameters.reserve(1 + FunctionDecl(fun).parameters().size());
    Type resType = resParam ? (Type)removeCatOrRef(TypeWithStorage(resParam->type))
                            : FunctionDecl(fun).resTypeNode().type();
    parameters.push_back(createTypeNode(ctx, loc, resType));
    for (size_t i = resParam ? 1 : 0; i < FunctionDecl(fun).parameters().size(); ++i) {
        parameters.push_back(createTypeNode(ctx, loc, FunctionDecl(fun).parameters()[i].type()));
    }
    Node* classCall = mkFunApplication(loc, mkIdentifier(loc, StringRef("FunctionPtr")),
            Feather_mkNodeList(loc, all(parameters)));
    Nest_setContext(classCall, ctx);
    if (!Nest_computeType(classCall))
        return nullptr;

    // Get the actual class object from the instantiation
    Type t = getType(classCall);

    // If the class is valid, we have a conversion
    if (t)
        return Feather_mkFunRef(loc, fun, Feather_mkTypeNode(loc, t));

    REP_ERROR(loc, "Invalid function: %1%") % callNode;
    return nullptr;
}

//! Get the number of parameters for a function-like decl
int getNumParams(Node* decl) {
    if (decl->nodeKind == nkFeatherDeclFunction) {
        Nest_NodeRange params = all(decl->children);
        return int(size(params)) - 2;
    }
    if (decl->nodeKind == nkSparrowDeclGenericFunction)
        return (int)size(genericFunParams(decl));
    if (decl->nodeKind == nkSparrowDeclSprFunction) {
        Node* parameters = at(decl->children, 0);
        return parameters ? (int)size(parameters->children) : 0;
    }
    return 0;
}

Node* _createFunPtrForDecl(Node* funNode) {
    const Location& loc = funNode->location;

    // Allow the funNode to return DeclExp
    Nest_setPropertyExplInt(funNode, propAllowDeclExp, 1);

    // Get all the declarations we may refer to
    Node* baseExp = nullptr;
    NodeVector decls = getDeclsFromNode(funNode, baseExp);
    auto declsEx = expandDecls(all(decls), funNode);

    if (baseExp)
        Nest_computeType(baseExp);

    // Filter out all the decls that cannot fit here
    //  - remove all the non-function decls
    //  - but keep generic functions as they are
    //  - if we have a base expression, make sure that the first parameter matches
    // put the good decls back in the 'decls' vector
    decls.clear();
    bool hasGenericFun = false;
    for (auto decl : declsEx) {
        decl = resultingDecl(decl);
        if (decl->nodeKind == nkSparrowDeclGenericFunction) {
            decls.push_back(decl);
            hasGenericFun = true;
        } else if (decl->nodeKind == nkFeatherDeclFunction) {
            // Check the first parameter if we arrived here through a baseExp
            if (baseExp && baseExp->type) {
                // Check parameter count
                size_t thisParamIdx = getResultParam(decl) ? 1 : 0;
                if (FunctionDecl(decl).parameters().size() <= thisParamIdx) {
                    continue;
                }

                // Ensure we can convert baseExp to the first param
                Type paramType = FunctionDecl(decl).parameters()[thisParamIdx].type();
                if (!g_ConvertService->checkConversion(
                            baseExp, paramType, flagDontCallConversionCtor)) {
                    continue;
                }
            }
            // The decl is ok
            decls.push_back(decl);
        }
        // Don't allow other decls
    }

    // Make sure we refer only to one decl
    if (decls.size() == 0) {
        REP_ERROR_RET(nullptr, loc, "No function found: %1%") % funNode;
    } else if (!hasGenericFun && decls.size() > 1) {
        REP_ERROR(loc, "Too many functions found while taking function reference: %1%") %
                decls.size();
        for (auto decl : decls)
            REP_INFO(decl->location, "See matching decl");
        return nullptr;
    } else if (hasGenericFun) {
        // We may have multiple decls that we are refering to, as we may have
        // multiple specializations of the generic
        // In this case, we just check that the argument count is the same
        auto numParams = getNumParams(decls[0]);
        bool mismatch = false;
        for (size_t i = 1; i < decls.size(); i++)
            if (getNumParams(decls[i]) != numParams) {
                mismatch = true;
                break;
            }
        if (mismatch) {
            REP_ERROR(loc, "Functions with different parameters count found while taking function "
                           "reference: %1%") %
                    decls.size();
            for (auto decl : decls)
                REP_INFO(decl->location, "See matching decl");
            return nullptr;
        }
    }
    Node* resDecl = decls[0]; // already obtained the resultingDecl from it
    Nest_freeNodeArray(declsEx);

    // Basic case: is this a plain function?
    if (!hasGenericFun) {
        return _createFunPtrForFeatherFun(resDecl, funNode);
    } else {
        // If we have a generic, try to wrap it in a lambda
        // TODO: In general we should create an object that is able to call any type of callable

        size_t numParams = size(genericFunParams(resDecl));

        Node* paramsType = mkIdentifier(loc, StringRef("AnyType"));

        NodeVector paramIds(numParams, nullptr);
        NodeVector args(numParams, nullptr);
        for (size_t i = 0; i < numParams; ++i) {
            string name = "p" + boost::lexical_cast<string>(i);
            paramIds[i] = mkSprParameter(loc, StringRef(name), paramsType, nullptr);
            args[i] = mkIdentifier(loc, StringRef(name));
        }

        Node* parameters = Feather_mkNodeList(loc, all(paramIds));
        Node* bodyExp = mkFunApplication(loc, funNode, Feather_mkNodeList(loc, all(args)));

        Node* res = mkLambdaExp(loc, parameters, nullptr, nullptr, bodyExp, nullptr);
        return res;
    }
}

Node* SprFrontend::createFunPtr(Node* funNode) {

    // Get rid of any node lists wraparounds
    ASSERT(funNode);
    while (funNode->nodeKind == nkFeatherNodeList && size(funNode->children) == 1) {
        if (!at(funNode->children, 0))
            break;
        funNode = at(funNode->children, 0);
    }

    // Check for fun applications
    if (funNode->nodeKind == nkSparrowExpFunApplication) {
        Node* featherFunCall = nullptr;

        // Compile the fun application node and try to extract the Feather fun call
        Nest_semanticCheck(funNode);
        Node* expl = Nest_explanation(funNode);
        if (!expl || !expl->type)
            return nullptr;

        // Case 1: directly expands into a Feather fun-call
        if (expl->nodeKind == nkFeatherExpFunCall) {
            featherFunCall = expl;
        }
        // Case 2: fun-call that returns an rvalue, wrapped in a node list with temp var
        if (expl->nodeKind == nkFeatherNodeList &&
                (size(expl->children) == 3 || size(expl->children) == 4)) {
            Node* varNode = at(expl->children, 0);
            Node* callNode = at(expl->children, 1);
            // we may have a destruct action in between
            Node* varRefNode = at(expl->children, size(expl->children) - 1);
            if (varNode && varNode->nodeKind == nkFeatherDeclVar && callNode &&
                    callNode->nodeKind == nkFeatherExpFunCall && varRefNode &&
                    varRefNode->nodeKind == nkFeatherExpVarRef)
                featherFunCall = callNode;
        }

        if (featherFunCall) {
            Node* referredFunDecl = at(featherFunCall->referredNodes, 0);
            return _createFunPtrForFeatherFun(referredFunDecl, funNode);
        } else
            return nullptr;
    }

    return _createFunPtrForDecl(funNode);
}
