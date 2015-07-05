#include <StdInc.h>
#include "FunApplication.h"
#include <NodeCommonsCpp.h>
#include <Nodes/Exp/Identifier.h>
#include <Helpers/Overload.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/Ct.h>
#include <Helpers/StdDef.h>

#include <Feather/Nodes/Decls/Class.h>
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

        if ( !arguments[0]->type() || !arguments[1]->type() )
            REP_INTERNAL(loc, "Invalid arguments");

        // Make sure the first argument is a type
        TypeRef t = getType(arguments[0]);
        if ( !t )
            REP_ERROR(arguments[0]->location(), "The first argument of a %1% must be a type") % castName;

        // Make sure that the second argument has storage
        if ( !arguments[1]->type()->hasStorage )
            REP_ERROR(arguments[1]->location(), "The second argument of a %1% must have a storage type (we have %2%)") % castName % arguments[1]->type();
        
        if ( secondShouldBeRef && arguments[1]->type()->numReferences == 0 )
            REP_ERROR(arguments[1]->location(), "The second argument of a %1% must be a reference (we have %2%)") % castName % arguments[1]->type();
    }
}

FunApplication::FunApplication(const Location& loc, Node* base, NodeList* arguments)
    : Node(loc, {base, arguments})
{
    if ( !arguments )
        children_[1] = mkNodeList(loc, {});
}

FunApplication::FunApplication(const Location& loc, Node* base, NodeVector args)
    : Node(loc, {base, mkNodeList(loc, move(args))})
{
}

Node* FunApplication::base() const
{
    ASSERT(children_.size() == 2);
    return children_[0];
}
NodeList* FunApplication::arguments() const
{
    ASSERT(children_.size() == 2);
    ASSERT(!children_[1] || children_[1]->nodeKind() == nkFeatherNodeList);
    return (NodeList*) children_[1];
}

void FunApplication::doSemanticCheck()
{
    ASSERT(children_.size() == 2);
    ASSERT(!children_[1] || children_[1]->nodeKind() == nkFeatherNodeList);
    Node* base = children_[0];
    NodeList* arguments = (NodeList*) children_[1];

    if ( !base )
        REP_INTERNAL(location_, "Don't know what function to call");
    
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
        arguments->semanticCheck();
    if ( arguments && arguments->hasError() )
        REP_INTERNAL(location_, "Args with error");

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
    NodeVector decls = getDeclsFromNode(base, thisArg);

    // If we didn't find any declarations, try the operator call
    if ( base->type()->hasStorage && decls.empty() )
    {
        Node* cls = classForTypeRaw(base->type());
        decls = cls->childrenContext()->currentSymTab()->lookupCurrent("()");
        if ( decls.empty() )
            REP_ERROR(location_, "Class %1% has no user defined call operators") % getName(cls);
        thisArg = base;
        functionName = "()";
    }

    // The name of function we are trying to call
    if ( functionName == base->nodeKindName() )
        functionName = "function";

    // The arguments to be used, including thisArg
    NodeVector args;
    if ( thisArg )
        args.push_back(thisArg);
    if ( arguments )
        args.insert(args.end(), arguments->children().begin(), arguments->children().end());

    // Check the right overload based on the type of the arguments
    EvalMode mode = context_->evalMode();
    if ( thisArg )
        mode = combineMode(thisArg->type()->mode, mode, location_, false);
    Node* res = selectOverload(context_, location_, mode, move(decls), args, true, functionName);

    setExplanation(res);
}

void FunApplication::checkStaticCast()
{
    NodeList* arguments = (NodeList*) children_[1];

    if ( !arguments )
        REP_ERROR(location_, "No arguments given to cast");

    // Check arguments
    checkCastArguments(location_, "cast", arguments->children());

    TypeRef destType = getType(arguments->children()[0]);
    TypeRef srcType = arguments->children()[1]->type();

    // Check if we can cast
    ConversionResult c = canConvert(arguments->children()[1], destType);
    if ( !c )
        REP_ERROR(location_, "Cannot cast from %1% to %2%; types are unrelated") % srcType % destType;
    Node* result = c.apply(arguments->children()[1]);
    result->setLocation(location_);

    setExplanation(result);
}

void FunApplication::checkReinterpretCast()
{
    NodeList* arguments = (NodeList*) children_[1];

    if ( !arguments )
        REP_ERROR(location_, "No arguments given to reinterpretCast");

    // Check arguments
    checkCastArguments(location_, "reinterpretCast", arguments->children(), true);

    TypeRef srcType = arguments->children()[1]->type();
    TypeRef destType = getType(arguments->children()[0]);
    ASSERT(destType);
    ASSERT(destType->hasStorage);
    if ( destType->numReferences == 0 )
        REP_ERROR(arguments->children()[0]->location(), "Destination type must be a reference (currently: %1%)") % destType;

    // If source is an l-value and the number of source reference is greater than the destination references, remove lvalue
    Node* arg = arguments->children()[1];
    if ( srcType->numReferences > destType->numReferences && srcType->typeKind == Nest::typeLValue )
        arg = mkMemLoad(arg->location(), arg);

    // Generate a bitcast operation out of this node
    setExplanation(mkBitcast(location_, mkTypeNode(arguments->children()[0]->location(), destType), arg));
}

void FunApplication::checkSizeOf()
{
    NodeList* arguments = (NodeList*) children_[1];

    if ( !arguments )
        REP_ERROR(location_, "No arguments given to sizeOf");

    // Make sure we have only one argument
    if ( arguments->children().size() != 1 )
        REP_ERROR(location_, "sizeOf expects one argument; %1% given") % arguments->children().size();
    Node* arg = arguments->children()[0];
    TypeRef t = arg->type();
    if ( !t )
        REP_INTERNAL(location_, "Invalid argument");

    // Make sure that the argument has storage, or it represents a type
    t = evalTypeIfPossible(arg);
    if ( !t->hasStorage )
    {
        REP_ERROR(arg->location(), "The argument of sizeOf must be a type or an expression with storage type (we have %1%)") % arg->type();
    }

    // Make sure the class that this refers to has the type properly computed
    Class* cls = classDecl(t);
    Node* mainNode = cls->childrenContext()->currentSymTab()->node();
    mainNode->computeType();

    // Remove l-value if we have some
    t = Feather::removeLValueIfPresent(t);

    // Get the size of the type of the argument
    uint64_t size = theCompiler().sizeOf(t);

    // Create a CtValue to hold the size
    setExplanation(mkCtValue(location_, StdDef::typeSizeType, &size));
}

void FunApplication::checkTypeOf()
{
    NodeList* arguments = (NodeList*) children_[1];

    if ( !arguments )
        REP_ERROR(location_, "No arguments given to typeOf");
    if ( arguments->children().size() != 1 )
        REP_ERROR(location_, "typeOf expects one argument; %1% given") % arguments->children().size();
    Node* arg = arguments->children()[0];

    // Compile the argument
    arguments->semanticCheck();

    // Make sure we have only one argument
    TypeRef t = arg->type();
    if ( !t )
        REP_INTERNAL(location_, "Invalid argument");
    t = Feather::removeLValueIfPresent(t);

    // Create a CtValue to hold the type
    setExplanation(createTypeNode(context_, arg->location(), t));
}

namespace
{
    bool checkIsValidImpl(const Location& loc, NodeList* arguments, const char* funName = "isValid", int numArgs = 1)
    {
        if ( !arguments )
            REP_ERROR(loc, "No arguments given to %1") % funName;

        // Make sure we have only one argument
        if ( arguments->children().size() != numArgs )
            REP_ERROR(loc, "%1% expects %2% arguments; %2% given") % funName % numArgs % arguments->children().size();

        // Try to compile the argument
        bool isValid = false;
        Nest::Common::DiagnosticSeverity level = Nest::theCompiler().diagnosticReporter().severityLevel();
        try
        {
            Nest::theCompiler().diagnosticReporter().setSeverityLevel(Nest::Common::diagInternalError);
            arguments->semanticCheck();
            isValid = !arguments->hasError() && !arguments->children()[0]->hasError();
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
    NodeList* arguments = (NodeList*) children_[1];

    bool isValid = checkIsValidImpl(location_, arguments, "isValid");

    // Create a CtValue to hold the result
    setExplanation(mkCtValue(location_, StdDef::typeBool, &isValid));
}

void FunApplication::checkIsValidAndTrue()
{
    NodeList* arguments = (NodeList*) children_[1];

    bool res = checkIsValidImpl(location_, arguments, "isValidAndTrue");
    if ( res )
    {
        Node* arg = arguments->children().front();
        arg->semanticCheck();

        // The expression must be CT
        if ( !isCt(arg) )
            REP_ERROR(location_, "ctEval expects an CT argument; %1% given") % arg->type();

        // Make sure we remove all the references
        const Location& loc = arg->location();
        size_t noRefs = arg->type()->numReferences;
        for ( size_t i=0; i<noRefs; ++i)
            arg = mkMemLoad(loc, arg);
        arg->setContext(context_);
        arg->semanticCheck();

        Node* val = theCompiler().ctEval(arg);
        if ( val->nodeKind() != nkFeatherExpCtValue )
            REP_ERROR(arg->location(), "Unknown value");

        // Get the value from the CtValue object
        res = getBoolCtValue(val);
    }

    // Create a CtValue to hold the result
    setExplanation(mkCtValue(location_, StdDef::typeBool, &res));
}

void FunApplication::checkValueIfValid()
{
    NodeList* arguments = (NodeList*) children_[1];

    bool isValid = checkIsValidImpl(location_, arguments, "valueIfValid", 2);

    setExplanation(isValid ? arguments->children()[0] : arguments->children()[1]);
}

void FunApplication::checkCtEval()
{
    NodeList* arguments = (NodeList*) children_[1];

    // Make sure we have one argument
    if ( arguments->children().size() != 1 )
        REP_ERROR(location_, "ctEval expects 1 argument; %1% given") % arguments->children().size();

    Node* arg = arguments->children().front();
    arg->semanticCheck();

    // The expression must be CT
    if ( !isCt(arg) )
        REP_ERROR(location_, "ctEval expects an CT argument; %1% given") % arg->type();

    // Make sure we remove all the references
    const Location& loc = arg->location();
    size_t noRefs = arg->type()->numReferences;
    for ( size_t i=0; i<noRefs; ++i)
        arg = mkMemLoad(loc, arg);
    arg->setContext(context_);
    arg->semanticCheck();

    Node* res = theCompiler().ctEval(arg);
    if ( res->nodeKind() != nkFeatherExpCtValue )
        REP_ERROR(arg->location(), "Unknown value");

    setExplanation(res);
}

void FunApplication::checkLift()
{
    NodeList* arguments = (NodeList*) children_[1];

    // Make sure we have one argument
    if ( arguments->children().size() != 1 )
        REP_ERROR(location_, "lift expects 1 argument; %1% given") % arguments->children().size();

    Node* arg = arguments->children().front();
    // Don't sematically check the argument; let the user of the lift to decide when it's better to do so

    // Create a construct of an AST node
    int* nodeHandle = (int*) arg;
    Node* base = mkCompoundExp(location_, mkIdentifier(location_, "Meta"), "AstNode");
    Node* arg1 = mkCtValue(location_, StdDef::typeRefInt, &nodeHandle);
    setExplanation( mkFunApplication(location_, base, {arg1}) );
}
