#include <StdInc.h>
#include "FunApplication.h"
#include <NodeCommonsCpp.h>
#include <Nodes/Exp/Identifier.h>
#include <Helpers/Overload.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/Ct.h>
#include <Helpers/StdDef.h>

#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;

namespace
{
    void checkCastArguments(const Location& loc, const char* castName, const NodeVector& arguments, bool secondShouldBeRef = false)
    {
        // Make sure we have only 2 arguments
        if ( arguments.size() != 2 )
            REP_ERROR(loc, "%1% expects 2 arguments; %2% given") % castName % arguments.size();

        if ( !arguments[0]->type || !arguments[1]->type )
            REP_INTERNAL(loc, "Invalid arguments");

        // Make sure the first argument is a type
        TypeRef t = getType(arguments[0]);
        if ( !t )
            REP_ERROR(arguments[0]->location, "The first argument of a %1% must be a type") % castName;

        // Make sure that the second argument has storage
        if ( !arguments[1]->type->hasStorage )
            REP_ERROR(arguments[1]->location, "The second argument of a %1% must have a storage type (we have %2%)") % castName % arguments[1]->type;
        
        if ( secondShouldBeRef && arguments[1]->type->numReferences == 0 )
            REP_ERROR(arguments[1]->location, "The second argument of a %1% must be a reference (we have %2%)") % castName % arguments[1]->type;
    }
}

FunApplication::FunApplication(const Location& loc, DynNode* base, Node* arguments)
    : DynNode(classNodeKind(), loc, {base, (DynNode*) arguments})
{
    ASSERT( !arguments || arguments->nodeKind == nkFeatherNodeList );
    if ( !arguments )
        data_.children[1] = mkNodeList(loc, {});
}

FunApplication::FunApplication(const Location& loc, DynNode* base, DynNodeVector args)
    : DynNode(classNodeKind(), loc, {base, (DynNode*) mkNodeList(loc, fromDyn(move(args)))})
{
}

DynNode* FunApplication::base() const
{
    ASSERT(data_.children.size() == 2);
    return (DynNode*) data_.children[0];
}
Node* FunApplication::arguments() const
{
    ASSERT(data_.children.size() == 2);
    ASSERT(!data_.children[1] || data_.children[1]->nodeKind == nkFeatherNodeList);
    return data_.children[1];
}

void FunApplication::doSemanticCheck()
{
    ASSERT(data_.children.size() == 2);
    ASSERT(!data_.children[1] || data_.children[1]->nodeKind == nkFeatherNodeList);
    DynNode* base = (DynNode*) data_.children[0];
    Node* arguments = data_.children[1];

    if ( !base )
        REP_INTERNAL(data_.location, "Don't know what function to call");
    
    // For the base expression allow it to return DeclExp
    base->setProperty(propAllowDeclExp, 1, true);

    // Compile the base expression
    // We can expect here both traditional expressions and nodes yielding decl-type types
    base->semanticCheck();

    // Check for Sparrow implicit functions
    Identifier* ident = base->as<Identifier>();
    if ( ident )
    {
        const string& id = ident->id();
        if ( id == "isValid" )
        {
            checkIsValid();
            return;
        }
        else if ( id == "isValidAndTrue" )
        {
            checkIsValidAndTrue();
            return;
        }
        else if ( id == "valueIfValid" )
        {
            checkValueIfValid();
            return;
        }
        else if ( id == "typeOf" )
        {
            checkTypeOf();
            return;
        }
        else if ( id == "lift" )
        {
            checkLift();
            return;
        }
    }

    // Compile the arguments
    if ( arguments )
        Nest::semanticCheck(arguments);
    if ( arguments && arguments->nodeError )
        REP_INTERNAL(data_.location, "Args with error");

    // Check for Sparrow implicit functions
    if ( ident )
    {
        const string& id = ident->id();
        if ( id == "cast" )
        {
            checkStaticCast();
            return;
        }
        else if ( id == "reinterpretCast" )
        {
            checkReinterpretCast();
            return;
        }
        else if ( id == "sizeOf" )
        {
            checkSizeOf();
            return;
        }
        else if ( id == "ctEval" )
        {
            checkCtEval();
            return;
        }
    }

    string functionName = base->toString();
    
    // Try to get the declarations pointed by the base node
    Node* thisArg = nullptr;
    NodeVector decls = getDeclsFromNode(base->node(), thisArg);

    // If we didn't find any declarations, try the operator call
    if ( base->type()->hasStorage && decls.empty() )
    {
        Node* cls = classForType(base->type());
        decls = cls->childrenContext->currentSymTab()->lookupCurrent("()");
        if ( decls.empty() )
            REP_ERROR(data_.location, "Class %1% has no user defined call operators") % getName(cls);
        thisArg = base->node();
        functionName = "()";
    }

    // The name of function we are trying to call
    if ( functionName == Nest::nodeKindName(base->node()) )
        functionName = "function";

    // The arguments to be used, including thisArg
    NodeVector args;
    if ( thisArg )
        args.push_back(thisArg);
    if ( arguments )
        args.insert(args.end(), arguments->children.begin(), arguments->children.end());

    // Check the right overload based on the type of the arguments
    EvalMode mode = data_.context->evalMode();
    if ( thisArg )
        mode = combineMode(thisArg->type->mode, mode, data_.location, false);
    Node* res = selectOverload(data_.context, data_.location, mode, move(decls), args, true, functionName);

    setExplanation(res);
}

void FunApplication::checkStaticCast()
{
    Node* arguments = data_.children[1];

    if ( !arguments )
        REP_ERROR(data_.location, "No arguments given to cast");

    // Check arguments
    checkCastArguments(data_.location, "cast", arguments->children);

    TypeRef destType = getType(arguments->children[0]);
    TypeRef srcType = arguments->children[1]->type;

    // Check if we can cast
    ConversionResult c = canConvert(arguments->children[1], destType);
    if ( !c )
        REP_ERROR(data_.location, "Cannot cast from %1% to %2%; types are unrelated") % srcType % destType;
    Node* result = c.apply(arguments->children[1]);
    result->location = data_.location;

    setExplanation(result);
}

void FunApplication::checkReinterpretCast()
{
    Node* arguments = data_.children[1];

    if ( !arguments )
        REP_ERROR(data_.location, "No arguments given to reinterpretCast");

    // Check arguments
    checkCastArguments(data_.location, "reinterpretCast", arguments->children, true);

    TypeRef srcType = arguments->children[1]->type;
    TypeRef destType = getType(arguments->children[0]);
    ASSERT(destType);
    ASSERT(destType->hasStorage);
    if ( destType->numReferences == 0 )
        REP_ERROR(arguments->children[0]->location, "Destination type must be a reference (currently: %1%)") % destType;

    // If source is an l-value and the number of source reference is greater than the destination references, remove lvalue
    Node* arg = arguments->children[1];
    if ( srcType->numReferences > destType->numReferences && srcType->typeKind == typeKindLValue )
        arg = mkMemLoad(arg->location, arg);

    // Generate a bitcast operation out of this node
    setExplanation(mkBitcast(data_.location, mkTypeNode(arguments->children[0]->location, destType), arg));
}

void FunApplication::checkSizeOf()
{
    Node* arguments = data_.children[1];

    if ( !arguments )
        REP_ERROR(data_.location, "No arguments given to sizeOf");

    // Make sure we have only one argument
    if ( arguments->children.size() != 1 )
        REP_ERROR(data_.location, "sizeOf expects one argument; %1% given") % arguments->children.size();
    Node* arg = arguments->children[0];
    TypeRef t = arg->type;
    if ( !t )
        REP_INTERNAL(data_.location, "Invalid argument");

    // Make sure that the argument has storage, or it represents a type
    t = evalTypeIfPossible(arg);
    if ( !t->hasStorage )
    {
        REP_ERROR(arg->location, "The argument of sizeOf must be a type or an expression with storage type (we have %1%)") % arg->type;
    }

    // Make sure the class that this refers to has the type properly computed
    Node* cls = classDecl(t);
    Node* mainNode = Nest::childrenContext(cls)->currentSymTab()->node();
    Nest::computeType(mainNode);

    // Remove l-value if we have some
    t = Feather::removeLValueIfPresent(t);

    // Get the size of the type of the argument
    uint64_t size = theCompiler().sizeOf(t);

    // Create a CtValue to hold the size
    setExplanation(mkCtValue(data_.location, StdDef::typeSizeType, &size));
}

void FunApplication::checkTypeOf()
{
    Node* arguments = data_.children[1];

    if ( !arguments )
        REP_ERROR(data_.location, "No arguments given to typeOf");
    if ( arguments->children.size() != 1 )
        REP_ERROR(data_.location, "typeOf expects one argument; %1% given") % arguments->children.size();
    Node* arg = arguments->children[0];

    // Compile the argument
    Nest::semanticCheck(arguments);

    // Make sure we have only one argument
    TypeRef t = arg->type;
    if ( !t )
        REP_INTERNAL(data_.location, "Invalid argument");
    t = Feather::removeLValueIfPresent(t);

    // Create a CtValue to hold the type
    setExplanation(createTypeNode(data_.context, arg->location, t));
}

namespace
{
    bool checkIsValidImpl(const Location& loc, Node* arguments, const char* funName = "isValid", int numArgs = 1)
    {
        if ( !arguments )
            REP_ERROR(loc, "No arguments given to %1") % funName;

        // Make sure we have only one argument
        if ( arguments->children.size() != numArgs )
            REP_ERROR(loc, "%1% expects %2% arguments; %2% given") % funName % numArgs % arguments->children.size();

        // Try to compile the argument
        bool isValid = false;
        Nest::Common::DiagnosticSeverity level = Nest::theCompiler().diagnosticReporter().severityLevel();
        try
        {
            Nest::theCompiler().diagnosticReporter().setSeverityLevel(Nest::Common::diagInternalError);
            Nest::semanticCheck(arguments);
            isValid = !arguments->nodeError && !arguments->children[0]->nodeError;
        }
        catch (...)
        {
        }
        Nest::theCompiler().diagnosticReporter().setSeverityLevel(level);
        return isValid;
    }
}

void FunApplication::checkIsValid()
{
    Node* arguments = data_.children[1];

    bool isValid = checkIsValidImpl(data_.location, arguments, "isValid");

    // Create a CtValue to hold the result
    setExplanation(mkCtValue(data_.location, StdDef::typeBool, &isValid));
}

void FunApplication::checkIsValidAndTrue()
{
    Node* arguments = data_.children[1];

    bool res = checkIsValidImpl(data_.location, arguments, "isValidAndTrue");
    if ( res )
    {
        Node* arg = arguments->children.front();
        Nest::semanticCheck(arg);

        // The expression must be CT
        if ( !isCt(arg) )
            REP_ERROR(data_.location, "ctEval expects an CT argument; %1% given") % arg->type;

        // Make sure we remove all the references
        const Location& loc = arg->location;
        size_t noRefs = arg->type->numReferences;
        for ( size_t i=0; i<noRefs; ++i)
            arg = mkMemLoad(loc, arg);
        Nest::setContext(arg, data_.context);
        Nest::semanticCheck(arg);

        Node* val = theCompiler().ctEval(arg);
        if ( val->nodeKind != nkFeatherExpCtValue )
            REP_ERROR(arg->location, "Unknown value");

        // Get the value from the CtValue object
        res = getBoolCtValue(val);
    }

    // Create a CtValue to hold the result
    setExplanation(mkCtValue(data_.location, StdDef::typeBool, &res));
}

void FunApplication::checkValueIfValid()
{
    Node* arguments = data_.children[1];

    bool isValid = checkIsValidImpl(data_.location, arguments, "valueIfValid", 2);

    setExplanation(isValid ? arguments->children[0] : arguments->children[1]);
}

void FunApplication::checkCtEval()
{
    Node* arguments = data_.children[1];

    // Make sure we have one argument
    if ( arguments->children.size() != 1 )
        REP_ERROR(data_.location, "ctEval expects 1 argument; %1% given") % arguments->children.size();

    Node* arg = arguments->children.front();
    Nest::semanticCheck(arg);

    // The expression must be CT
    if ( !isCt(arg) )
        REP_ERROR(data_.location, "ctEval expects an CT argument; %1% given") % arg->type;

    // Make sure we remove all the references
    const Location& loc = arg->location;
    size_t noRefs = arg->type->numReferences;
    for ( size_t i=0; i<noRefs; ++i)
        arg = mkMemLoad(loc, arg);
    Nest::setContext(arg, data_.context);
    Nest::semanticCheck(arg);

    DynNode* res = (DynNode*) theCompiler().ctEval(arg);
    if ( res->nodeKind() != nkFeatherExpCtValue )
        REP_ERROR(arg->location, "Unknown value");

    setExplanation(res);
}

void FunApplication::checkLift()
{
    Node* arguments = data_.children[1];

    // Make sure we have one argument
    if ( arguments->children.size() != 1 )
        REP_ERROR(data_.location, "lift expects 1 argument; %1% given") % arguments->children.size();

    Node* arg = arguments->children.front();
    // Don't semantically check the argument; let the user of the lift to decide when it's better to do so

    // Create a construct of an AST node
    int* nodeHandle = (int*) arg;
    Node* base = mkCompoundExp(data_.location, mkIdentifier(data_.location, "Meta"), "AstNode");
    Node* arg1 = mkCtValue(data_.location, StdDef::typeRefInt, &nodeHandle);
    setExplanation( mkFunApplication(data_.location, base, NodeVector(1, arg1)) );
}
