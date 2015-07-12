#include <StdInc.h>
#include "LambdaFunction.h"
#include <Helpers/SprTypeTraits.h>

#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/Decl.h>

#include <Nest/Frontend/SourceCode.h>

using namespace SprFrontend;
using namespace Feather;

LambdaFunction::LambdaFunction(const Location& loc, NodeList* parameters, DynNode* returnType, DynNode* body, NodeList* closureParams)
    : DynNode(classNodeKind(), loc, {}, {parameters, returnType, body, closureParams})
{
}

void LambdaFunction::dump(ostream& os) const
{
    ASSERT(data_.referredNodes.size() == 4);
    NodeList* parameters = (NodeList*) data_.referredNodes[0];
    DynNode* returnType = (DynNode*) data_.referredNodes[1];
    DynNode* body = (DynNode*) data_.referredNodes[2];
    NodeList* closureParams = (NodeList*) data_.referredNodes[3];
    os << "(fun";
    os << " " << parameters;
    if ( closureParams )
        os << ".{" << closureParams << "}";
    if ( returnType )
        os << ": " << returnType;
    if ( body )
        os << " " << body;
}

void LambdaFunction::doSemanticCheck()
{
    ASSERT(data_.referredNodes.size() == 4);
    NodeList* parameters = (NodeList*) data_.referredNodes[0];
    DynNode* returnType = (DynNode*) data_.referredNodes[1];
    DynNode* body = (DynNode*) data_.referredNodes[2];
    NodeList* closureParams = (NodeList*) data_.referredNodes[3];

    Function* parentFun = getParentFun(data_.context);
    CompilationContext* parentContext = parentFun ? parentFun->context() : context();

    NodeList* ctorArgs = nullptr;
    NodeList* ctorParams = nullptr;

    // Create the enclosing class body node list
    NodeList* classBody = mkNodeList(data_.location, {});

    // The actual enclosed function
    classBody->addChild(mkSprFunction(data_.location, "()", parameters, returnType, body));

    // Add a private default ctor
    classBody->addChild(mkSprFunction(data_.location, "ctor", nullptr, nullptr, mkLocalSpace(data_.location, DynNodeVector()), nullptr, privateAccess));

    // For each closure variable, create:
    // - a member variable in the class
    // - an initialization line in the ctor
    // - a parameter to ctor
    // - an argument to pass to the ctor
    DynNodeVector ctorStmts;
    if ( closureParams )
    {
        DynNodeVector ctorArgsNodes, ctorParamsNodes;
        for ( DynNode* arg: closureParams->children() )
        {
            if ( !arg || arg->nodeKind() != nkSparrowExpIdentifier )
                REP_INTERNAL(arg->location(), "The closure parameter must be identifier");
            const string& varName = getName(arg);
            const Location& loc = arg->location();

            // Create an argument node to pass to the ctor
            arg->setContext(data_.context);
            arg->semanticCheck();
            ctorArgsNodes.push_back(arg);

            // Create a closure parameter
            TypeRef varType = removeLValueIfPresent(arg->type());
            ctorParamsNodes.push_back(mkSprParameter(loc, varName, varType));

            // Create a similar variable in the enclosing class - must have the same name
            classBody->addChild(mkSprVariable(loc, varName, varType, nullptr, privateAccess));

            // Create an initialization for the variable
            DynNode* fieldRef = mkCompoundExp(loc, mkThisExp(loc), varName);
            DynNode* paramRef = mkIdentifier(loc, varName);
            const char* op = (varType->numReferences == 0) ? "ctor" : ":=";
            DynNode* initCall = mkOperatorCall(loc, fieldRef, op, paramRef);
            ctorStmts.push_back(initCall);
        }
        ctorArgs = mkNodeList(data_.location, move(ctorArgsNodes));
        ctorParams = mkNodeList(data_.location, move(ctorParamsNodes));
    }

    // Create the ctor used to initialize the closure class
    DynNode* ctorBody = mkLocalSpace(data_.location, ctorStmts);
    DynNode* enclosingCtor = mkSprFunction(data_.location, "ctor", ctorParams, nullptr, ctorBody);
    enclosingCtor->setProperty(propNoDefault, 1);
    classBody->addChild(enclosingCtor);

    // Create the lambda closure
    DynNode* closure = mkSprClass(data_.location, "$lambdaEnclosure", nullptr, nullptr, nullptr, classBody);

    // Add the closure as a top level node of this node
    closure->setContext(parentContext);  // Put the enclosing class in the context of the parent function
    ASSERT(parentContext->sourceCode());
    parentContext->sourceCode()->addAdditionalNode(closure->node());

    // Compute the type for the enclosing class
    closure->computeType();
    Node* cls = closure->explanation()->node();
    ASSERT(cls);

    // Make sure the closure class is semantically checked
    closure->semanticCheck();

    // Create a resulting object: a constructor call to our class
    DynNode* classId = (DynNode*) createTypeNode(data_.context, data_.location, getDataType(cls));
    setExplanation(mkFunApplication(data_.location, classId, ctorArgs));
}
