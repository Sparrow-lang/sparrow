#include <StdInc.h>
#include "LambdaFunction.h"
#include <Helpers/SprTypeTraits.h>

#include <Feather/Util/Context.h>
#include <Feather/Util/Decl.h>

#include <Nest/Frontend/SourceCode.h>

using namespace SprFrontend;
using namespace Feather;

LambdaFunction::LambdaFunction(const Location& loc, Node* parameters, DynNode* returnType, DynNode* body, Node* closureParams)
    : DynNode(classNodeKind(), loc, {}, {(DynNode*) parameters, returnType, body, (DynNode*) closureParams})
{
    ASSERT( !parameters || parameters->nodeKind == nkFeatherNodeList );
    ASSERT( !closureParams || closureParams->nodeKind == nkFeatherNodeList );
}

void LambdaFunction::dump(ostream& os) const
{
    ASSERT(data_.referredNodes.size() == 4);
    Node* parameters = data_.referredNodes[0];
    DynNode* returnType = (DynNode*) data_.referredNodes[1];
    DynNode* body = (DynNode*) data_.referredNodes[2];
    Node* closureParams = data_.referredNodes[3];
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
    Node* parameters = data_.referredNodes[0];
    Node* returnType = data_.referredNodes[1];
    Node* body = data_.referredNodes[2];
    Node* closureParams = data_.referredNodes[3];

    Node* parentFun = getParentFun(data_.context);
    CompilationContext* parentContext = parentFun ? parentFun->context : context();

    Node* ctorArgs = nullptr;
    Node* ctorParams = nullptr;

    // Create the enclosing class body node list
    Node* classBody = mkNodeList(data_.location, {});

    // The actual enclosed function
    classBody->children.push_back(mkSprFunction(data_.location, "()", parameters, returnType, body));

    // Add a private default ctor
    classBody->children.push_back(mkSprFunction(data_.location, "ctor", nullptr, nullptr, mkLocalSpace(data_.location, {}), nullptr, privateAccess));

    // For each closure variable, create:
    // - a member variable in the class
    // - an initialization line in the ctor
    // - a parameter to ctor
    // - an argument to pass to the ctor
    NodeVector ctorStmts;
    if ( closureParams )
    {
        NodeVector ctorArgsNodes, ctorParamsNodes;
        for ( Node* arg: closureParams->children )
        {
            if ( !arg || arg->nodeKind != nkSparrowExpIdentifier )
                REP_INTERNAL(arg->location, "The closure parameter must be identifier");
            const string& varName = getName(arg);
            const Location& loc = arg->location;

            // Create an argument node to pass to the ctor
            Nest::setContext(arg, data_.context);
            Nest::semanticCheck(arg);
            ctorArgsNodes.push_back(arg);

            // Create a closure parameter
            TypeRef varType = removeLValueIfPresent(arg->type);
            ctorParamsNodes.push_back(mkSprParameter(loc, varName, varType));

            // Create a similar variable in the enclosing class - must have the same name
            classBody->children.push_back(mkSprVariable(loc, varName, varType, nullptr, privateAccess));

            // Create an initialization for the variable
            Node* fieldRef = mkCompoundExp(loc, mkThisExp(loc), varName);
            Node* paramRef = mkIdentifier(loc, varName);
            const char* op = (varType->numReferences == 0) ? "ctor" : ":=";
            Node* initCall = mkOperatorCall(loc, fieldRef, op, paramRef);
            ctorStmts.push_back(initCall);
        }
        ctorArgs = mkNodeList(data_.location, move(ctorArgsNodes));
        ctorParams = mkNodeList(data_.location, move(ctorParamsNodes));
    }

    // Create the ctor used to initialize the closure class
    Node* ctorBody = mkLocalSpace(data_.location, ctorStmts);
    Node* enclosingCtor = mkSprFunction(data_.location, "ctor", ctorParams, nullptr, ctorBody);
    Nest::setProperty(enclosingCtor, propNoDefault, 1);
    classBody->children.push_back(enclosingCtor);

    // Create the lambda closure
    Node* closure = mkSprClass(data_.location, "$lambdaEnclosure", nullptr, nullptr, nullptr, classBody);

    // Add the closure as a top level node of this node
    Nest::setContext(closure, parentContext);  // Put the enclosing class in the context of the parent function
    ASSERT(parentContext->sourceCode());
    parentContext->sourceCode()->addAdditionalNode(closure);

    // Compute the type for the enclosing class
    Nest::computeType(closure);
    Node* cls = Nest::explanation(closure);
    ASSERT(cls);

    // Make sure the closure class is semantically checked
    Nest::semanticCheck(closure);

    // Create a resulting object: a constructor call to our class
    Node* classId = createTypeNode(data_.context, data_.location, getDataType(cls));
    setExplanation(mkFunApplication(data_.location, classId, ctorArgs));
}
