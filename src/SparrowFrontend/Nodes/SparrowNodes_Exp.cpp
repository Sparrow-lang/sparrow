#include <StdInc.h>
#include "SparrowNodes.h"

#include <SparrowFrontendTypes.h>

#include <Helpers/CommonCode.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/Ct.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/StdDef.h>
#include <Helpers/Convert.h>
#include <Helpers/Overload.h>

#include <Feather/Util/StringData.h>
#include <Feather/Util/Decl.h>
#include <Feather/Util/Context.h>

#include <Nest/Frontend/SourceCode.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace
{
    ////////////////////////////////////////////////////////////////////////////
    // Helpers for Identifier and CompoundExp nodes
    //

    Node* getIdentifierResult(CompilationContext* ctx, const Location& loc, const NodeVector& decls, Node* baseExp, bool allowDeclExp)
    {
        // If this points to one declaration only, try to use that declaration
        if ( decls.size() == 1 )
        {
            Node* resDecl = resultingDecl(decls[0]);
            ASSERT(resDecl);
            
            // Check if we can refer to a variable
            if ( resDecl->nodeKind == nkFeatherDeclVar )
            {
                if ( isField(resDecl) )
                {
                    if ( !baseExp )
                        REP_INTERNAL(loc, "No base expression to refer to a field");

                    // Make sure the base is a reference
                    if ( baseExp->type->numReferences == 0 )
                    {
                        ConversionResult res = canConvert(baseExp, addRef(baseExp->type));
                        if ( !res )
                            REP_INTERNAL(loc, "Cannot add reference to base of field access");
                        baseExp = res.apply(baseExp);
                        if ( !Nest_computeType(baseExp) )
                            return nullptr;
                    }
                    Node* baseCvt = nullptr;
                    doDereference1(baseExp, baseCvt);  // ... but no more than one reference
                    baseExp = baseCvt;
                    
                    return mkFieldRef(loc, baseExp, resDecl);
                }
                return mkVarRef(loc, resDecl);
            }
            
            // If this is a using, get its value
            if ( resDecl->nodeKind == nkSparrowDeclUsing )
                return at(resDecl->children, 0);

            // Try to convert this to a type
            TypeRef t = nullptr;
            Node* cls = Nest_ofKind(resDecl, nkFeatherDeclClass);
            if ( cls )
                t = getDataType(cls);
            if ( resDecl->nodeKind == nkSparrowDeclSprConcept )
                t = getConceptType(resDecl);
            if ( t )
                return createTypeNode(ctx, loc, t);

        }

        // Add the referenced declarations as a property to our result
        if ( allowDeclExp )
            return mkDeclExp(loc, decls, baseExp);

        // If we are here, this identifier could only represent a function application
        Node* fapp = mkFunApplication(loc, mkDeclExp(loc, decls, baseExp), nullptr);
        Nest_setContext(fapp, ctx);
        return Nest_semanticCheck(fapp);
    }


    ////////////////////////////////////////////////////////////////////////////
    // Helpers for FunApplication node
    //

    bool checkCastArguments(const Location& loc, const char* castName, NodeRange arguments, bool secondShouldBeRef = false)
    {
        // Make sure we have only 2 arguments
        auto numArgs = Nest_nodeRangeSize(arguments);
        if ( numArgs != 2 )
        {
            REP_ERROR(loc, "%1% expects 2 arguments; %2% given") % castName % numArgs;
            return false;
        }
        Node* arg1 = at(arguments, 0);
        Node* arg2 = at(arguments, 1);

        if ( !arg1->type || !arg2->type )
            REP_INTERNAL(loc, "Invalid arguments");

        // Make sure the first argument is a type
        TypeRef t = getType(arg1);
        if ( !t )
        {
            REP_ERROR(arg1->location, "The first argument of a %1% must be a type") % castName;
            return false;
        }

        // Make sure that the second argument has storage
        if ( !arg2->type->hasStorage )
        {
            REP_ERROR(arg2->location, "The second argument of a %1% must have a storage type (we have %2%)") % castName % arg2->type;
            return false;
        }
        
        if ( secondShouldBeRef && arg2->type->numReferences == 0 )
        {
            REP_ERROR(arg2->location, "The second argument of a %1% must be a reference (we have %2%)") % castName % arg2->type;
            return false;
        }
        return true;
    }

    Node* checkStaticCast(Node* node)
    {
        Node* arguments = at(node->children, 1);

        if ( !arguments )
            REP_ERROR_RET(nullptr, node->location, "No arguments given to cast");

        // Check arguments
        if ( !checkCastArguments(node->location, "cast", all(arguments->children)) )
            return nullptr;

        TypeRef destType = getType(at(arguments->children, 0));
        TypeRef srcType = at(arguments->children, 1)->type;

        // Check if we can cast
        ConversionResult c = canConvert(at(arguments->children, 1), destType);
        if ( !c )
            REP_ERROR_RET(nullptr, node->location, "Cannot cast from %1% to %2%; types are unrelated") % srcType % destType;
        Node* result = c.apply(at(arguments->children, 1));
        result->location = node->location;

        return result;
    }

    Node* checkReinterpretCast(Node* node)
    {
        Node* arguments = at(node->children, 1);

        if ( !arguments )
            REP_ERROR_RET(nullptr, node->location, "No arguments given to reinterpretCast");

        // Check arguments
        if ( !checkCastArguments(node->location, "reinterpretCast", all(arguments->children), true) )
            return nullptr;

        TypeRef srcType = at(arguments->children, 1)->type;
        TypeRef destType = getType(at(arguments->children, 0));
        ASSERT(destType);
        ASSERT(destType->hasStorage);
        if ( destType->numReferences == 0 )
            REP_ERROR_RET(nullptr, at(arguments->children, 0)->location, "Destination type must be a reference (currently: %1%)") % destType;

        // If source is an l-value and the number of source reference is greater than the destination references, remove lvalue
        Node* arg = at(arguments->children, 1);
        if ( srcType->numReferences > destType->numReferences && srcType->typeKind == typeKindLValue )
            arg = mkMemLoad(arg->location, arg);

        // Generate a bitcast operation out of this node
        return mkBitcast(node->location, mkTypeNode(at(arguments->children, 0)->location, destType), arg);
    }

    Node* checkSizeOf(Node* node)
    {
        Node* arguments = at(node->children, 1);

        if ( !arguments )
            REP_ERROR_RET(nullptr, node->location, "No arguments given to sizeOf");

        // Make sure we have only one argument
        if ( Nest_nodeArraySize(arguments->children) != 1 )
            REP_ERROR_RET(nullptr, node->location, "sizeOf expects one argument; %1% given") % Nest_nodeArraySize(arguments->children);
        Node* arg = at(arguments->children, 0);
        TypeRef t = arg->type;
        if ( !t )
            REP_INTERNAL(node->location, "Invalid argument");

        // Make sure that the argument has storage, or it represents a type
        t = evalTypeIfPossible(arg);
        if ( !t->hasStorage )
        {
            REP_ERROR_RET(nullptr, arg->location, "The argument of sizeOf must be a type or an expression with storage type (we have %1%)") % arg->type;
        }

        // Make sure the class that this refers to has the type properly computed
        Node* cls = classDecl(t);
        Node* mainNode = Nest_childrenContext(cls)->currentSymTab->node;
        if ( !Nest_computeType(mainNode) )
            return nullptr;

        // Remove l-value if we have some
        t = Feather::removeLValueIfPresent(t);

        // Get the size of the type of the argument
        uint64_t size = theCompiler().sizeOf(t);

        // Create a CtValue to hold the size
        return mkCtValue(node->location, StdDef::typeSizeType, &size);
    }

    Node* checkTypeOf(Node* node)
    {
        Node* arguments = at(node->children, 1);

        if ( !arguments )
            REP_ERROR_RET(nullptr, node->location, "No arguments given to typeOf");
        if ( Nest_nodeArraySize(arguments->children) != 1 )
            REP_ERROR_RET(nullptr, node->location, "typeOf expects one argument; %1% given") % Nest_nodeArraySize(arguments->children);
        Node* arg = at(arguments->children, 0);

        // Compile the argument
        if ( !Nest_semanticCheck(arguments) )
            return nullptr;

        // Make sure we have only one argument
        TypeRef t = arg->type;
        if ( !t )
            REP_INTERNAL(node->location, "Invalid argument");
        t = Feather::removeLValueIfPresent(t);

        // Create a CtValue to hold the type
        return createTypeNode(node->context, arg->location, t);
    }

    bool checkIsValidImpl(const Location& loc, Node* arguments, const char* funName = "isValid", int numArgs = 1)
    {
        if ( !arguments )
        {
            REP_ERROR(loc, "No arguments given to %1") % funName;
            return false;
        }

        // Make sure we have only one argument
        if ( Nest_nodeArraySize(arguments->children) != numArgs )
        {
            REP_ERROR(loc, "%1% expects %2% arguments; %2% given") % funName % numArgs % Nest_nodeArraySize(arguments->children);
            return false;
        }

        // Try to compile the argument
        bool isValid = false;
        int oldVal = Nest_isReportingEnabled();
        Nest_enableReporting(0);
        Node* res = Nest_semanticCheck(arguments);
        isValid = res != nullptr
                && !arguments->nodeError
                && !at(arguments->children, 0)->nodeError
                && Nest_getSuppressedErrorsNum() == 0;
        Nest_enableReporting(oldVal);
        return isValid;
    }

    Node* checkIsValid(Node* node)
    {
        Node* arguments = at(node->children, 1);

        bool isValid = checkIsValidImpl(node->location, arguments, "isValid");

        // Create a CtValue to hold the result
        return mkCtValue(node->location, StdDef::typeBool, &isValid);
    }

    Node* checkIsValidAndTrue(Node* node)
    {
        Node* arguments = at(node->children, 1);

        bool res = checkIsValidImpl(node->location, arguments, "isValidAndTrue");
        Node* arg = nullptr;
        if ( res )
        {
            arg = at(arguments->children, 0);
            if ( !Nest_semanticCheck(arg) ) 
                res = false;
        }

        // The expression must be CT
        if ( res && !isCt(arg) )
        {
            REP_ERROR(node->location, "ctEval expects an CT argument; %1% given") % arg->type;
            res = false;
        }

        if ( res )
        {
            // Make sure we remove all the references
            const Location& loc = arg->location;
            size_t noRefs = arg->type->numReferences;
            for ( size_t i=0; i<noRefs; ++i)
                arg = mkMemLoad(loc, arg);
            Nest_setContext(arg, node->context);
            if ( !Nest_semanticCheck(arg) )
                res = false;
        }

        Node* val = nullptr;
        if ( res )
        {
            val = theCompiler().ctEval(arg);
            if ( val->nodeKind != nkFeatherExpCtValue )
            {
                REP_ERROR(arg->location, "Unknown value");
                res = false;
            }
        }

        // Get the value from the CtValue object
        if ( res )
            res = getBoolCtValue(val);

        // Create a CtValue to hold the result
        return mkCtValue(node->location, StdDef::typeBool, &res);
    }

    Node* checkValueIfValid(Node* node)
    {
        Node* arguments = at(node->children, 1);

        bool isValid = checkIsValidImpl(node->location, arguments, "valueIfValid", 2);

        return isValid ? at(arguments->children, 0) : at(arguments->children, 1);
    }

    Node* checkCtEval(Node* node)
    {
        Node* arguments = at(node->children, 1);

        // Make sure we have one argument
        if ( Nest_nodeArraySize(arguments->children) != 1 )
            REP_ERROR_RET(nullptr, node->location, "ctEval expects 1 argument; %1% given") % Nest_nodeArraySize(arguments->children);

        Node* arg = at(arguments->children, 0);
        if ( !Nest_semanticCheck(arg) )
            return nullptr;

        // The expression must be CT
        if ( !isCt(arg) )
            REP_ERROR_RET(nullptr, node->location, "ctEval expects an CT argument; %1% given") % arg->type;

        // Make sure we remove all the references
        const Location& loc = arg->location;
        size_t noRefs = arg->type->numReferences;
        for ( size_t i=0; i<noRefs; ++i)
            arg = mkMemLoad(loc, arg);
        Nest_setContext(arg, node->context);
        if ( !Nest_semanticCheck(arg) )
            return nullptr;

        Node* res = theCompiler().ctEval(arg);
        if ( res->nodeKind != nkFeatherExpCtValue )
            REP_ERROR_RET(nullptr, arg->location, "Unknown value");

        return res;
    }

    Node* checkLift(Node* node)
    {
        Node* arguments = at(node->children, 1);

        // Make sure we have one argument
        if ( Nest_nodeArraySize(arguments->children) != 1 )
            REP_ERROR_RET(nullptr, node->location, "lift expects 1 argument; %1% given") % Nest_nodeArraySize(arguments->children);

        Node* arg = at(arguments->children, 0);
        // Don't semantically check the argument; let the user of the lift to decide when it's better to do so

        // Create a construct of an AST node
        int* nodeHandle = (int*) arg;
        Node* base = mkCompoundExp(node->location, mkIdentifier(node->location, "Meta"), "AstNode");
        Node* arg1 = mkCtValue(node->location, StdDef::typeRefInt, &nodeHandle);
        return  mkFunApplication(node->location, base, NodeVector(1, arg1)) ;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Helpers for OperatorCall node
    //

    Node* trySelectOperator(const string& operation, const NodeVector& args, CompilationContext* searchContext, bool searchOnlyGivenContext,
        CompilationContext* callContext, const Location& callLocation, EvalMode mode)
    {
        SymTab* sTab = searchContext->currentSymTab;
        NodeVector decls = searchOnlyGivenContext ? Nest_symTabLookupCurrent(sTab, operation.c_str()) : Nest_symTabLookup(sTab, operation.c_str());
        if ( !decls.empty() )
            return selectOverload(callContext, callLocation, mode, move(decls), args, false, operation);

        return nullptr;
    }

    #define CHECK_RET(expr) \
        { \
            Node* ret = expr; \
            if ( ret ) return ret; \
        }

    Node* selectOperator(Node* node, const string& operation, Node* arg1, Node* arg2)
    {
        NodeVector args;
        if ( arg1 )
            args.push_back(arg1);
        if ( arg2 )
            args.push_back(arg2);

        // If this is an unary operator, try using the operation prefix
        string opPrefix;
        if ( arg1 && !arg2 )
            opPrefix = "post_";
        if ( !arg1 && arg2 )
            opPrefix = "pre_";

        // Identifiy the first valid operand, so that we can search near it
        Node* base = nullptr;
        if ( arg1 )
            base = arg1;
        else if ( arg2 )
            base = arg2;

        Node* argClass = nullptr;
        if ( base )
        {
            if ( !Nest_semanticCheck(base) )
                return nullptr;
            argClass = classForType(base->type);
        }

        EvalMode mode;
        if ( argClass )
        {
            mode = base->type->mode;

            // Step 1: Try to find an operator that match in the class of the base expression
            if ( !opPrefix.empty() )
                CHECK_RET(trySelectOperator(opPrefix + operation, args, Nest_childrenContext(argClass), true, node->context, node->location, mode));
            CHECK_RET(trySelectOperator(operation, args, Nest_childrenContext(argClass), true, node->context, node->location, mode));

            // Step 2: Try to find an operator that match in the near the class the base expression
            mode = node->context->evalMode;
            if ( !opPrefix.empty() )
                CHECK_RET(trySelectOperator(opPrefix + operation, args, argClass->context, true, node->context, node->location, mode));
            CHECK_RET(trySelectOperator(operation, args, argClass->context, true, node->context, node->location, mode));
        }

        // Step 3: General search from the current context
        mode = node->context->evalMode;
        if ( !opPrefix.empty() )
            CHECK_RET(trySelectOperator(opPrefix + operation, args, node->context, false, node->context, node->location, mode));
        CHECK_RET(trySelectOperator(operation, args, node->context, false, node->context, node->location, mode));

        return nullptr;
    }

    Node* checkConvertNullToRefByte(Node* orig)
    {
        if ( isSameTypeIgnoreMode(orig->type, StdDef::typeNull) )
        {
            Node* res = mkNull(orig->location, mkTypeNode(orig->location, StdDef::typeRefByte));
            Nest_setContext(res, orig->context);
            return Nest_computeType(res) ? res : nullptr;
        }
        return orig;
    }

    Node* handleFApp(Node* node)
    {
        Node* arg1 = at(node->children, 0);
        Node* arg2 = at(node->children, 1);

        if ( arg2 && arg2->nodeKind != nkFeatherNodeList )
            REP_INTERNAL(arg2->location, "Expected node list for function application; found %1%") % arg2;

        return mkFunApplication(node->location, arg1, arg2);
    }

    Node* handleDotExpr(Node* node)
    {
        Node* arg1 = at(node->children, 0);
        Node* arg2 = at(node->children, 1);

        if ( arg2->nodeKind != nkSparrowExpIdentifier )
            REP_INTERNAL(arg2->location, "Expected identifier after dot; found %1%") % arg2;

        return mkCompoundExp(node->location, arg1, Nest_toString(arg2));
    }

    Node* handleRefEq(Node* node)
    {
        Node* arg1 = at(node->children, 0);
        Node* arg2 = at(node->children, 1);

        if ( !Nest_semanticCheck(arg1) || !Nest_semanticCheck(arg2) )
            return nullptr;

        // If we have null as arguments, convert them to "RefByte"
        arg1 = checkConvertNullToRefByte(arg1);
        arg2 = checkConvertNullToRefByte(arg2);

        // Make sure that both the arguments are references
        if ( arg1->type->numReferences == 0 )
            REP_ERROR_RET(nullptr, node->location, "Left operand of a reference equality operator is not a reference (%1%)") % arg1->type;
        if ( arg2->type->numReferences == 0 )
            REP_ERROR_RET(nullptr, node->location, "Right operand of a reference equality operator is not a reference (%1%)") % arg2->type;

        Node* arg1Cvt = nullptr;
        Node* arg2Cvt = nullptr;
        doDereference1(arg1, arg1Cvt);             // Dereference until the last reference
        doDereference1(arg2, arg2Cvt);
        arg1Cvt = mkBitcast(node->location, mkTypeNode(node->location, StdDef::typeRefByte), arg1Cvt);
        arg2Cvt = mkBitcast(node->location, mkTypeNode(node->location, StdDef::typeRefByte), arg2Cvt);

        return mkFunCall(node->location, StdDef::opRefEq, {arg1Cvt, arg2Cvt});
    }

    Node* handleRefNe(Node* node)
    {
        Node* arg1 = at(node->children, 0);
        Node* arg2 = at(node->children, 1);

        if ( !Nest_semanticCheck(arg1) || !Nest_semanticCheck(arg2) )
            return nullptr;

        // If we have null as arguments, convert them to "RefByte"
        arg1 = checkConvertNullToRefByte(arg1);
        arg2 = checkConvertNullToRefByte(arg2);

        // Make sure that both the arguments are references
        if ( arg1->type->numReferences == 0 )
            REP_ERROR_RET(nullptr, node->location, "Left operand of a reference equality operator is not a reference (%1%)") % arg1->type;
        if ( arg2->type->numReferences == 0 )
            REP_ERROR_RET(nullptr, node->location, "Right operand of a reference equality operator is not a reference (%1%)") % arg2->type;

        Node* arg1Cvt = nullptr;
        Node* arg2Cvt = nullptr;
        doDereference1(arg1, arg1Cvt);             // Dereference until the last reference
        doDereference1(arg2, arg2Cvt);
        arg1Cvt = mkBitcast(node->location, mkTypeNode(node->location, StdDef::typeRefByte), arg1Cvt);
        arg2Cvt = mkBitcast(node->location, mkTypeNode(node->location, StdDef::typeRefByte), arg2Cvt);

        return mkFunCall(node->location, StdDef::opRefNe, {arg1Cvt, arg2Cvt});
    }

    Node* handleRefAssign(Node* node)
    {
        Node* arg1 = at(node->children, 0);
        Node* arg2 = at(node->children, 1);

        if ( !Nest_semanticCheck(arg1) || !Nest_semanticCheck(arg2) )
            return nullptr;

        // Make sure the first argument is a reference reference
        if ( arg1->type->numReferences < 2 )
            REP_ERROR_RET(nullptr, node->location, "Left operand of a reference assign operator is not a reference reference (%1%)") % arg1->type;
        TypeRef arg1BaseType = Feather::removeRef(arg1->type);

        // Check the second type to be null or a reference
        TypeRef arg2Type = arg2->type;
        if ( !Feather::isSameTypeIgnoreMode(arg2Type, StdDef::typeNull) )
        {
            if ( arg2Type->numReferences == 0 )
                REP_ERROR_RET(nullptr, node->location, "Right operand of a reference assign operator is not a reference (%1%)") % arg2->type;
        }

        // Check for a conversion from the second argument to the first argument
        ConversionResult c = canConvert(arg2, arg1BaseType);
        if ( !c )
            REP_ERROR_RET(nullptr, node->location, "Cannot convert from %1% to %2%") % arg2->type % arg1BaseType;
        Node* cvt = c.apply(arg2);

        // Return a memstore operator
        return mkMemStore(node->location, cvt, arg1);
    }

    Node* handleFunPtr(Node* node)
    {
        Node* funNode = at(node->children, 1);

        return createFunPtr(funNode);
    }

    string argTypeStr(Node* node)
    {
        return node && node->type ? node->type->description : "?";
    }

    ////////////////////////////////////////////////////////////////////////////
    // Helpers for InfixExp node
    //

    static const char* operPropName = "spr.operation";
    
    const string& getOperation(Node* infixExp)
    {
        return Nest_getCheckPropertyString(infixExp, operPropName);
    }

    // Visual explanation:
    //
    //     X     <- this        Y     <- keep this pointing here
    //    / \                  / \
    //   Y   R           =>   L   X
    //  / \                      / \
    // L   M                    M   R
    //
    // We must keep the this pointer in the same position, but we must switch the arguments & operations
    // The left argument of this will become the right argument of this
    void swapLeft(Node* node)
    {
        Node* other = at(node->children, 0);
        ASSERT(other->nodeKind == nkSparrowExpInfixExp);

        at(node->children, 0) = at(other->children, 0);
        at(other->children, 0) = at(other->children, 1);
        at(other->children, 1) = at(node->children, 1);
        at(node->children, 1) = other;

        string otherOper = getOperation(other);
        Nest_setProperty(other, operPropName, getOperation(node));
        Nest_setProperty(node, operPropName, move(otherOper));
    }

    // Visual explanation:
    //
    //     X     <- this        Y     <- keep this pointing here
    //    / \                  / \
    //   L   Y           =>   X   R
    //      / \              / \
    //     M   R            L   M    
    //
    // We must keep the this pointer in the same position, but we must switch the arguments & operations
    // The right argument of this will become the left argument of this
    //
    // NOTE: this function will move 'M' from left-side to right-side
    //       it cannot be applied if 'Y' is a unary operator that contains a null 'M'
    void swapRight(Node* node)
    {
        Node* other = at(node->children, 1);
        ASSERT(other->nodeKind == nkSparrowExpInfixExp);

        at(node->children, 1) = at(other->children, 1);
        at(other->children, 1) = at(other->children, 0);
        at(other->children, 0) = at(node->children, 0);
        at(node->children, 0) = other;

        string otherOper = getOperation(other);
        Nest_setProperty(other, operPropName, getOperation(node));
        Nest_setProperty(node, operPropName, move(otherOper));
    }

    int getIntValue(Node* node, const NodeVector& decls, int defaultVal)
    {
        // If no declarations found, return the default value
        if ( decls.empty() )
            return defaultVal;

        // If more than one declarations found, issue a warning
        if ( decls.size() > 1 )
        {
            REP_WARNING(node->location, "Multiple precedence declarations found for '%1%'") % getOperation(node);
            for ( Node* decl: decls )
                if ( decl )
                    REP_INFO(decl->location, "See alternative");
        }

        // Just one found. Evaluate its value
        Node* n = decls.front();
        if ( !Nest_semanticCheck(n) )
            return defaultVal;
        if ( n->nodeKind == nkSparrowDeclUsing )
            n = at(n->children, 0);

        return getIntCtValue(Nest_explanation(n));
    }

    int getPrecedence(Node* node)
    {
        Node* arg1 = at(node->children, 0);

        // For prefix operator, search with a special name
        string oper = arg1 ? getOperation(node) : "__pre__";

        string precedenceName = "oper_precedence_" + oper;
        string defaultPrecedenceName = "oper_precedence_default";

        // Perform a name lookup for the actual precedence name
        int res = getIntValue(node, Nest_symTabLookup(node->context->currentSymTab, precedenceName.c_str()), -1);
        if ( res > 0 )
            return res;

        // Search the default precedence name
        res = getIntValue(node, Nest_symTabLookup(node->context->currentSymTab, defaultPrecedenceName.c_str()), -1);
        if ( res > 0 )
            return res;

        return 0;
    }

    bool isRightAssociativity(Node* node)
    {
        string assocName = "oper_assoc_" + getOperation(node);

        // Perform a name lookup for the actual associativity name
        int res = getIntValue(node, Nest_symTabLookup(node->context->currentSymTab, assocName.c_str()), 1);
        return res < 0;
    }

    void handlePrecedence(Node* node)
    {
        ASSERT(node && node->nodeKind == nkSparrowExpInfixExp);

        // We go from left to right, trying to fix the precedence
        // Usually the tree is is deep on the left side, but it can contain some sub-trees on the right side too
        for ( ;/*ever*/; )
        {
            int rankCur = getPrecedence(node);

            // Check right wing first
            Node* rightOp = Nest_ofKind(at(node->children, 1), nkSparrowExpInfixExp);
            if ( rightOp )
            {
                int rankRight = getPrecedence(rightOp);
                // We never swap right if on the right we have a prefix operator (no 1st arg)
                if ( rankRight <= rankCur && at(rightOp->children, 0) )
                {
                    swapRight(node);
                    continue;
                }
            }


            Node* leftOp = Nest_ofKind(at(node->children, 0), nkSparrowExpInfixExp);
            if ( leftOp )
            {
                handlePrecedence(leftOp);

                int rankLeft = getPrecedence(leftOp);
                if ( rankLeft < rankCur )
                {
                    swapLeft(node);
                    // leftOp is now on right of this node, containing the operation of this node
                    // make sure the precedence is ok on the right side too
                    handlePrecedence(leftOp);
                    continue;
                }
            }

            // Nothing to do anymore
            break;
        }
    }

    void handleAssociativity(Node* node)
    {
        // Normally, we should only check the left children for right associativity.
        // But, as we apply precedence, we must check for associativity in both directions

        ASSERT(node && node->nodeKind == nkSparrowExpInfixExp);

        const string& curOp = getOperation(node);
        bool isRightAssoc = isRightAssociativity(node);
        if ( !isRightAssoc )
        {
            for ( ;/*ever*/; )
            {
                Node* arg2 = at(node->children, 1);
                if ( !arg2 || arg2->nodeKind != nkSparrowExpInfixExp )
                    break;

                handleAssociativity(arg2);
                // We never swap right if on the right we have a prefix operator (no 1st arg)
                if ( arg2 && curOp == getOperation(arg2) && at(arg2->children, 0) )
                    swapRight(node);
                else
                    break;
            }
        }
        else
        {
            for ( ;/*ever*/; )
            {
                Node* arg1 = at(node->children, 0);
                if ( !arg1 || arg1->nodeKind != nkSparrowExpInfixExp )
                    break;

                handleAssociativity(arg1);
                if ( arg1 && curOp == getOperation(arg1) )
                    swapLeft(node);
                else
                    break;
            }
        }
    }
}


Node* Literal_SemanticCheck(Node* node)
{
    const string& litType = Nest_getCheckPropertyString(node, "spr.literalType");
    const string& data = Nest_getCheckPropertyString(node, "spr.literalData");

    // Get the type of the literal by looking up the type name
    Node* ident = mkIdentifier(node->location, litType);
    Nest_setContext(ident, node->context);
    if ( !Nest_computeType(ident) )
        return nullptr;
    TypeRef t = getType(ident);
    t = Feather::changeTypeMode(t, modeCt, node->location);
    
    if ( litType == "StringRef" )
    {
        // Create the explanation
        Feather::StringData s = Feather::StringData::copyStdString(data);
        return Feather::mkCtValue(node->location, t, &s);
    }
    else
        return Feather::mkCtValue(node->location, t, data);
}

Node* This_SemanticCheck(Node* node)
{
    return mkIdentifier(node->location, "$this");
}

Node* Identifier_SemanticCheck(Node* node)
{
    const string& id = Nest_getCheckPropertyString(node, "name");

    // Search in the current symbol table for the identifier
    NodeVector decls = Nest_symTabLookup(node->context->currentSymTab, id.c_str());
    if ( decls.empty() )
        REP_ERROR_RET(nullptr, node->location, "No declarations found with the given name (%1%)") % id;

    // If at least one decl is a field or method, then transform this into a compound expression starting from 'this'
    bool needsThis = false;
    for ( Node* decl: decls )
    {
        if ( !Nest_computeType(decl) )
            continue;
        Node* expl = Nest_explanation(decl);
        if ( isField(expl) )
        {
            needsThis = true;
            break;
        }
        if ( decl && decl->nodeKind == nkSparrowDeclSprFunction && funHasThisParameters(decl) )
        {
            needsThis = true;
            break;
        }
    }
    if ( needsThis )
    {
        // Add 'this' parameter to handle this case
        Node* res = mkCompoundExp(node->location, mkThisExp(node->location), id);
        return res;
    }

    bool allowDeclExp = 0 != Nest_getCheckPropertyInt(node, propAllowDeclExp);
    Node* res = getIdentifierResult(node->context, node->location, move(decls), nullptr, allowDeclExp);
    ASSERT(res);
    return res;
}

Node* CompoundExp_SemanticCheck(Node* node)
{
    Node* base = at(node->children, 0);
    const string& id = Nest_getCheckPropertyString(node, "name");

    // For the base expression allow it to return DeclExp
    Nest_setProperty(base, propAllowDeclExp, 1, true);

    // Compile the base expression
    // We can expect at the base node both traditional expressions and nodes yielding decl-type types
    if ( !Nest_semanticCheck(base) )
        return nullptr;

    // Try to get the declarations pointed by the base node
    Node* baseDataExp = nullptr;
    NodeVector baseDecls = getDeclsFromNode(base, baseDataExp);
    
    // If the base has storage, retain it as the base data expression
    if ( baseDecls.empty() && base->type->hasStorage )
        baseDataExp = base;
    if ( baseDataExp && !Nest_computeType(baseDataExp) )
        return nullptr;
    Nest_setProperty(node, "baseDataExp", baseDataExp);

    // Get the declarations that this node refers to
    NodeVector decls;
    if ( !baseDecls.empty() )
    {
        // Get the referred declarations; search for our id inside the symbol table of the declarations of the base
        for ( Node* baseDecl: baseDecls )
        {
            NodeVector declsCur = Nest_symTabLookupCurrent(baseDecl->childrenContext->currentSymTab, id.c_str());
            decls.insert(decls.end(), declsCur.begin(), declsCur.end());
        }
    }
    else if ( base->type->hasStorage )
    {
        // If the base is an expression with a data type, treat this as a data access
        Node* classDecl = classForType(base->type);
        if ( !Nest_computeType(classDecl) )
            return nullptr;

        // Search for a declaration in the class 
        decls = Nest_symTabLookupCurrent(classDecl->childrenContext->currentSymTab, id.c_str());
    }

    if ( decls.empty() )
        REP_ERROR_RET(nullptr, node->location, "No declarations found with the name '%1%' inside %2%: %3%") % id % base % base->type;

    
    bool allowDeclExp = 0 != Nest_getCheckPropertyInt(node, propAllowDeclExp);
    Node* res = getIdentifierResult(node->context, node->location, move(decls), baseDataExp, allowDeclExp);
    ASSERT(res);
    return res;
}

Node* FunApplication_SemanticCheck(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 2);
    ASSERT(!at(node->children, 1) || at(node->children, 1)->nodeKind == nkFeatherNodeList);
    Node* base = at(node->children, 0);
    Node* arguments = at(node->children, 1);

    if ( !base )
        REP_INTERNAL(node->location, "Don't know what function to call");
    
    // For the base expression allow it to return DeclExp
    Nest_setProperty(base, propAllowDeclExp, 1, true);

    // Compile the base expression
    // We can expect here both traditional expressions and nodes yielding decl-type types
    if ( !Nest_semanticCheck(base) )
        return nullptr;

    // Check for Sparrow implicit functions
    Node* ident = nullptr;
    if ( base->nodeKind == nkSparrowExpIdentifier )
    {
        ident = base;
        const string& id = getName(ident);
        if ( id == "isValid" )
        {
            return checkIsValid(node);
        }
        else if ( id == "isValidAndTrue" )
        {
            return checkIsValidAndTrue(node);
        }
        else if ( id == "valueIfValid" )
        {
            return checkValueIfValid(node);
        }
        else if ( id == "typeOf" )
        {
            return checkTypeOf(node);
        }
        else if ( id == "lift" )
        {
            return checkLift(node);
        }
    }

    // Compile the arguments
    if ( arguments && !Nest_semanticCheck(arguments) )
        return nullptr;
    if ( arguments && arguments->nodeError )
        REP_INTERNAL(node->location, "Args with error");

    // Check for Sparrow implicit functions
    if ( ident )
    {
        const string& id = getName(ident);
        if ( id == "cast" )
        {
            return checkStaticCast(node);
        }
        else if ( id == "reinterpretCast" )
        {
            return checkReinterpretCast(node);
        }
        else if ( id == "sizeOf" )
        {
            return checkSizeOf(node);
        }
        else if ( id == "ctEval" )
        {
            return checkCtEval(node);
        }
    }

    string functionName = Nest_toString(base);
    
    // Try to get the declarations pointed by the base node
    Node* thisArg = nullptr;
    NodeVector decls = getDeclsFromNode(base, thisArg);

    // If we didn't find any declarations, try the operator call
    if ( base->type->hasStorage && decls.empty() )
    {
        Node* cls = classForType(base->type);
        decls = Nest_symTabLookupCurrent(cls->childrenContext->currentSymTab, "()");
        if ( decls.empty() )
            REP_ERROR_RET(nullptr, node->location, "Class %1% has no user defined call operators") % getName(cls);
        thisArg = base;
        functionName = "()";
    }

    // The name of function we are trying to call
    if ( functionName == Nest_nodeKindName(base) )
        functionName = "function";

    // The arguments to be used, including thisArg
    NodeVector args;
    if ( thisArg )
        args.push_back(thisArg);
    if ( arguments )
        args.insert(args.end(), arguments->children.beginPtr, arguments->children.endPtr);

    // Check the right overload based on the type of the arguments
    EvalMode mode = node->context->evalMode;
    if ( thisArg )
        mode = combineMode(thisArg->type->mode, mode, node->location, false);
    Node* res = selectOverload(node->context, node->location, mode, move(decls), args, true, functionName);

    return res;
}

Node* OperatorCall_SemanticCheck(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 2);
    Node* arg1 = at(node->children, 0);
    Node* arg2 = at(node->children, 1);
    const string& operation = getOperation(node);

    if ( arg1 && arg2 )
    {
        if ( operation == "__dot__" )
        {
            return handleDotExpr(node);
        }
        if ( operation == "===" )
        {
            return handleRefEq(node);
        }
        else if ( operation == "!==" )
        {
            return handleRefNe(node);
        }
        else if ( operation == ":=" )
        {
            return handleRefAssign(node);
        }
    }
    if ( arg1 && operation == "__fapp__" )
    {
        return handleFApp(node);
    }
    if ( arg2 && !arg1 && operation == "\\" )
    {
        return handleFunPtr(node); // TODO: this should ideally be defined in std lib
    }

    // Search for the operator
    Node* res = selectOperator(node, operation, arg1, arg2);

    // If nothing found try fall-back.
    if ( !res && arg1 && arg2 )
    {
        string op1, op2;
        Node* a1 = arg1;
        Node* a2 = arg2;

        if ( operation == "!=" )  // Transform '!=' into '=='
        {
            op1 = "==";
            op2 = "!";
        }
        else if ( operation == ">" )    // Transform '>' into '<'
        {
            op1 = "<";
            a1 = arg2;
            a2 = arg1;
        }
        else if ( operation == "<=" )   // Transform '<=' into '<'
        {
            op1 = "<";
            op2 = "!";
            a1 = arg2;
            a2 = arg1;
        }
        else if ( operation == ">=" ) // Transform '>=' into '<'
        {
            op1 = "<";
            op2 = "!";
        }

        // Check for <op>= operators
        else if ( operation.size() >= 2 && operation[operation.size()-1] == '=' )
        {
            string baseOperation = operation.substr(0, operation.size()-1);

            // Transform 'a <op>= b' into 'a = a <op> b'
            op1 = operation.substr(0, operation.size()-1);
            op2 = "=";
        }

        if ( !op1.empty() )
        {
            Node* r = selectOperator(node, op1, a1, a2);
            if ( r )
            {
                if ( !Nest_semanticCheck(r) )
                    return nullptr;
                if ( op2 == "!" )
                    res = selectOperator(node, op2, r, nullptr);
                else if ( op2 == "=" )
                    res = selectOperator(node, op2, a1, r);
                else if ( op2.empty() )
                    res = r;
            }
        }
    }

    if ( !res )
    {
        if ( arg1 && arg2 )
            REP_ERROR_RET(nullptr, node->location, "Cannot find an overload for calling operator %2% %1% %3%") % operation % argTypeStr(arg1) % argTypeStr(arg2);
        else if ( arg1 )
            REP_ERROR_RET(nullptr, node->location, "Cannot find an overload for calling operator %2% %1%") % operation % argTypeStr(arg1);
        else if ( arg2 )
            REP_ERROR_RET(nullptr, node->location, "Cannot find an overload for calling operator %1% %2%") % operation % argTypeStr(arg2);
        else 
            REP_ERROR_RET(nullptr, node->location, "Cannot find an overload for calling operator %1%") % operation;
    }

    return res;
}

Node* InfixExp_SemanticCheck(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 2);
    Node* arg1 = at(node->children, 0);
    Node* arg2 = at(node->children, 1);

    // This is constructed in such way that left most operations are applied first.
    // This way, we have a tree that has a lot of children on the left side and one children on the right side
    // When applying the precedence and associativity rules for the expressions in here, we will try to balance this
    // tree, and move some elements to the right side of the tree by performing some swaps

    handlePrecedence(node);
    handleAssociativity(node);

    arg1 = at(node->children, 0);
    arg2 = at(node->children, 1);

    return mkOperatorCall(node->location, arg1, getOperation(node), arg2);
}

Node* LambdaFunction_SemanticCheck(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->referredNodes) == 4);
    Node* parameters = at(node->referredNodes, 0);
    Node* returnType = at(node->referredNodes, 1);
    Node* body = at(node->referredNodes, 2);
    Node* closureParams = at(node->referredNodes, 3);

    Node* parentFun = getParentFun(node->context);
    CompilationContext* parentContext = parentFun ? parentFun->context : node->context;

    Node* ctorArgs = nullptr;
    Node* ctorParams = nullptr;

    // Create the enclosing class body node list
    Node* classBody = mkNodeList(node->location, {});

    // The actual enclosed function
    Nest_appendNodeToArray(&classBody->children, mkSprFunction(node->location, "()", parameters, returnType, body));

    // Add a private default ctor
    Nest_appendNodeToArray(&classBody->children, mkSprFunction(node->location, "ctor", nullptr, nullptr, mkLocalSpace(node->location, {}), nullptr, privateAccess));

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
            Nest_setContext(arg, node->context);
            if ( !Nest_semanticCheck(arg) )
                return nullptr;
            ctorArgsNodes.push_back(arg);

            // Create a closure parameter
            TypeRef varType = removeLValueIfPresent(arg->type);
            ctorParamsNodes.push_back(mkSprParameter(loc, varName, varType));

            // Create a similar variable in the enclosing class - must have the same name
            Nest_appendNodeToArray(&classBody->children, mkSprVariable(loc, varName, varType, nullptr, privateAccess));

            // Create an initialization for the variable
            Node* fieldRef = mkCompoundExp(loc, mkThisExp(loc), varName);
            Node* paramRef = mkIdentifier(loc, varName);
            const char* op = (varType->numReferences == 0) ? "ctor" : ":=";
            Node* initCall = mkOperatorCall(loc, fieldRef, op, paramRef);
            ctorStmts.push_back(initCall);
        }
        ctorArgs = mkNodeList(node->location, move(ctorArgsNodes));
        ctorParams = mkNodeList(node->location, move(ctorParamsNodes));
    }

    // Create the ctor used to initialize the closure class
    Node* ctorBody = mkLocalSpace(node->location, ctorStmts);
    Node* enclosingCtor = mkSprFunction(node->location, "ctor", ctorParams, nullptr, ctorBody);
    Nest_setProperty(enclosingCtor, propNoDefault, 1);
    Nest_appendNodeToArray(&classBody->children, enclosingCtor);

    // Create the lambda closure
    Node* closure = mkSprClass(node->location, "$lambdaEnclosure", nullptr, nullptr, nullptr, classBody);

    // Add the closure as a top level node of this node
    Nest_setContext(closure, parentContext);  // Put the enclosing class in the context of the parent function
    ASSERT(parentContext->sourceCode);
    Nest_appendNodeToArray(&parentContext->sourceCode->additionalNodes, closure);

    // Compute the type for the enclosing class
    if ( !Nest_computeType(closure) )
        return nullptr;
    Node* cls = Nest_explanation(closure);
    ASSERT(cls);

    // Make sure the closure class is semantically checked
    if ( !Nest_semanticCheck(closure) )
        return nullptr;

    // Create a resulting object: a constructor call to our class
    Node* classId = createTypeNode(node->context, node->location, getDataType(cls));
    return mkFunApplication(node->location, classId, ctorArgs);
}

Node* SprConditional_SemanticCheck(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 3);
    Node* cond = at(node->children, 0);
    Node* alt1 = at(node->children, 1);
    Node* alt2 = at(node->children, 2);

    if ( !Nest_semanticCheck(alt1) || !Nest_semanticCheck(alt2) )
        return nullptr;

    TypeRef t1 = alt1->type;
    TypeRef t2 = alt2->type;

    // Get the common type
    TypeRef resType = commonType(node->context, t1, t2);
    if ( resType == StdDef::typeVoid )
        REP_ERROR_RET(nullptr, node->location, "Cannot deduce the result type for a conditional expression (%1%, %2%)") % t1 % t2;
    
    // Convert both types to the result type
    ConversionResult c1  = canConvertType(node->context, t1, resType);
    ConversionResult c2  = canConvertType(node->context, t2, resType);

    alt1 = c1.apply(node->context, alt1);
    alt2 = c2.apply(node->context, alt2);

    return Feather::mkConditional(node->location, cond, alt1, alt2);
}

Node* DeclExp_SemanticCheck(Node* node)
{
    // Make sure we computed the type for all the referred nodes
    for ( Node* n: node->referredNodes )
    {
        if ( n && !Nest_computeType(n) )
            continue;
    }
    node->type = Feather::getVoidType(node->context->evalMode);
    return node;    // This node should never be translated directly
}

Node* StarExp_SemanticCheck(Node* node)
{
    Node* base = at(node->children, 0);

    // For the base expression allow it to return DeclExp
    Nest_setProperty(base, propAllowDeclExp, 1, true);

    // Get the declarations from the base expression
    if ( !Nest_semanticCheck(base) )
        return nullptr;            
    Node* baseExp;
    NodeVector baseDecls = getDeclsFromNode(base, baseExp);
    if ( baseDecls.empty() )
        REP_ERROR_RET(nullptr, base->location, "Invalid expression inside star expression (no referred declarations)");

    // Get the referred declarations
    NodeVector decls;
    for ( Node* baseDecl: baseDecls )
    {
        if ( !baseDecl )
            continue;

        // Get the sym tab from the base declaration
        SymTab* baseSymTab = Nest_childrenContext(baseDecl)->currentSymTab;
        if ( !baseSymTab )
            continue;

        // Get all the symbols from the symbol table

        // Search in the symbol table of the base for the identifier
        decls = Nest_symTabAllEntries(baseSymTab);
    }

    if ( decls.empty() )
        REP_ERROR(node->location, "No declarations found with the star expression");
    
    // This expands to a declaration expression
    return mkDeclExp(node->location, move(decls));
}
