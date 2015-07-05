#include <StdInc.h>
#include "LambdaFunction.h"
#include <Helpers/SprTypeTraits.h>

#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/Decl.h>

#include <Nest/Frontend/SourceCode.h>

using namespace SprFrontend;
using namespace Feather;

LambdaFunction::LambdaFunction(const Location& loc, NodeList* parameters, Node* returnType, Node* body, NodeList* closureParams)
    : Node(loc, {}, {parameters, returnType, body, closureParams})
{
}

void LambdaFunction::dump(ostream& os) const
{
    ASSERT(referredNodes_.size() == 4);
    NodeList* parameters = (NodeList*) referredNodes_[0];
    Node* returnType = referredNodes_[1];
    Node* body = referredNodes_[2];
    NodeList* closureParams = (NodeList*) referredNodes_[3];
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
    ASSERT(referredNodes_.size() == 4);
    NodeList* parameters = (NodeList*) referredNodes_[0];
    Node* returnType = referredNodes_[1];
    Node* body = referredNodes_[2];
    NodeList* closureParams = (NodeList*) referredNodes_[3];

    Function* parentFun = getParentFun(context_);
    CompilationContext* parentContext = parentFun ? parentFun->context() : context();

    NodeList* ctorArgs = nullptr;
    NodeList* ctorParams = nullptr;

    // Create the enclosing class body node list
    NodeList* classBody = mkNodeList(location_, {});

    // The actual enclosed function
    classBody->addChild(mkSprFunction(location_, "()", parameters, returnType, body));

    // Add a private default ctor
    classBody->addChild(mkSprFunction(location_, "ctor", nullptr, nullptr, mkLocalSpace(location_, NodeVector()), nullptr, privateAccess));

    // For each closure variable, create:
    // - a member variable in the class
    // - an initialization line in the ctor
    // - a parameter to ctor
    // - an argument to pass to the ctor
    NodeVector ctorStmts;
    if ( closureParams )
    {
        NodeVector ctorArgsNodes, ctorParamsNodes;
        for ( Node* arg: closureParams->children() )
        {
            if ( !arg || arg->nodeKind() != nkSparrowExpIdentifier )
                REP_INTERNAL(arg->location(), "The closure parameter must be identifier");
            const string& varName = getName(arg);
            const Location& loc = arg->location();

            // Create an argument node to pass to the ctor
            arg->setContext(context_);
            arg->semanticCheck();
            ctorArgsNodes.push_back(arg);

            // Create a closure parameter
            Type* varType = removeLValueIfPresent(arg->type());
            ctorParamsNodes.push_back(mkSprParameter(loc, varName, varType));

            // Create a similar variable in the enclosing class - must have the same name
            classBody->addChild(mkSprVariable(loc, varName, varType, nullptr, privateAccess));

            // Create an initialization for the variable
            Node* fieldRef = mkCompoundExp(loc, mkThisExp(loc), varName);
            Node* paramRef = mkIdentifier(loc, varName);
            const char* op = (varType->noReferences() == 0) ? "ctor" : ":=";
            Node* initCall = mkOperatorCall(loc, fieldRef, op, paramRef);
            ctorStmts.push_back(initCall);
        }
        ctorArgs = mkNodeList(location_, move(ctorArgsNodes));
        ctorParams = mkNodeList(location_, move(ctorParamsNodes));
    }

    // Create the ctor used to initialize the closure class
    Node* ctorBody = mkLocalSpace(location_, ctorStmts);
    Node* enclosingCtor = mkSprFunction(location_, "ctor", ctorParams, nullptr, ctorBody);
    enclosingCtor->setProperty(propNoDefault, 1);
    classBody->addChild(enclosingCtor);

    // Create the lambda closure
    Node* closure = mkSprClass(location_, "$lambdaEnclosure", nullptr, nullptr, nullptr, classBody);

    // Add the closure as a top level node of this node
    closure->setContext(parentContext);  // Put the enclosing class in the context of the parent function
    ASSERT(parentContext->sourceCode());
    parentContext->sourceCode()->addAdditionalNode(closure);

    // Compute the type for the enclosing class
    closure->computeType();
    Class* cls = (Feather::Class*) closure->explanation();
    ASSERT(cls);

    // Make sure the closure class is semantically checked
    closure->semanticCheck();

    // Create a resulting object: a constructor call to our class
    Node* classId = (Node*) createTypeNode(context_, location_, Type::fromBasicType(getDataType(cls)));
    setExplanation(mkFunApplication(location_, classId, ctorArgs));
}
