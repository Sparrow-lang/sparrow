#include <StdInc.h>
#include "SparrowNodes.h"
#include "Builder.h"

#include <SparrowFrontendTypes.h>
#include <SprDebug.h>

#include <Helpers/CommonCode.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/Ct.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/StdDef.h>
#include <Helpers/Convert.h>
#include <Helpers/Overload.h>

#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Api/SourceCode.h"

using namespace SprFrontend;
using namespace Nest;

namespace
{
    ////////////////////////////////////////////////////////////////////////////
    // Helpers for Identifier and CompoundExp nodes
    //

    Node* getIdentifierResult(Node* node, NodeRange decls, Node* baseExp, bool allowDeclExp)
    {
        const Location& loc = node->location;

        // If this points to one declaration only, try to use that declaration
        if ( Nest_nodeRangeSize(decls) == 1 )
        {
            Node* resDecl = resultingDecl(at(decls, 0));
            ASSERT(resDecl);

            // Check if we can refer to a variable
            if ( resDecl->nodeKind == nkFeatherDeclVar )
            {
                if ( isField(resDecl) )
                {
                    if ( !baseExp )
                        REP_INTERNAL(loc, "No base expression to refer to a field");
                    ASSERT(baseExp);

                    // Make sure the base is a reference
                    if ( baseExp->type->numReferences == 0 )
                    {
                        ConversionResult res = canConvert(baseExp, Feather_addRef(baseExp->type));
                        if ( !res )
                            REP_INTERNAL(loc, "Cannot add reference to base of field access");
                        baseExp = res.apply(baseExp);
                        if ( !Nest_computeType(baseExp) )
                            return nullptr;
                    }
                    Node* baseCvt = nullptr;
                    doDereference1(baseExp, baseCvt);  // ... but no more than one reference
                    baseExp = baseCvt;

                    return Feather_mkFieldRef(loc, baseExp, resDecl);
                }
                return Feather_mkVarRef(loc, resDecl);
            }

            // If this is a using, get its value
            if ( resDecl->nodeKind == nkSparrowDeclUsing )
                return at(resDecl->children, 0);

            // If we allow decl expressions, create a decl expression out of this
            if ( allowDeclExp )
                return mkDeclExp(loc, decls, baseExp);

            // Try to convert this to a type
            TypeRef t = nullptr;
            if ( resDecl->nodeKind == nkFeatherDeclClass )
                t = Feather_getDataType(resDecl, 0, modeRtCt);
            if ( resDecl->nodeKind == nkSparrowDeclSprConcept || resDecl->nodeKind == nkSparrowDeclGenericClass )
                t = getConceptType(resDecl);
            if ( t )
                return createTypeNode(node->context, loc, t);
        }

        // If we allow decl expressions, create a decl expression out of this
        if ( allowDeclExp )
            return mkDeclExp(loc, decls, baseExp);

        // If we are here, this identifier could only represent a function application
        Node* fapp = mkFunApplication(loc, mkDeclExp(loc, decls, baseExp), nullptr);
        Nest_setContext(fapp, node->context);
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
        ASSERT(arg1->type && arg2->type);

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
            arg = Feather_mkMemLoad(arg->location, arg);

        // Generate a bitcast operation out of this node
        return Feather_mkBitcast(node->location, Feather_mkTypeNode(at(arguments->children, 0)->location, destType), arg);
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
        Node* cls = Feather_classDecl(t);
        Node* mainNode = Nest_childrenContext(cls)->currentSymTab->node;
        if ( !Nest_computeType(mainNode) )
            return nullptr;

        // Remove l-value if we have some
        t = Feather_removeLValueIfPresent(t);

        // Get the size of the type of the argument
        uint64_t size = Nest_sizeOf(t);

        // Create a CtValue to hold the size
        return Feather_mkCtValueT(node->location, StdDef::typeSizeType, &size);
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
        t = Feather_removeLValueIfPresent(t);

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
        if ( (int) Nest_nodeArraySize(arguments->children) != numArgs )
        {
            REP_ERROR(loc, "%1% expects %2% arguments; %3% given") % funName % numArgs % Nest_nodeArraySize(arguments->children);
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
        return Feather_mkCtValueT(node->location, StdDef::typeBool, &isValid);
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
        if ( res && !Feather_isCt(arg) )
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
                arg = Feather_mkMemLoad(loc, arg);
            Nest_setContext(arg, node->context);
            if ( !Nest_semanticCheck(arg) )
                res = false;
        }

        Node* val = nullptr;
        if ( res )
        {
            val = Nest_ctEval(arg);
            if ( val->nodeKind != nkFeatherExpCtValue )
            {
                REP_ERROR(arg->location, "Unknown value");
                res = false;
            }
        }

        // Get the value from the CtValue object
        if ( res )
            res = SprFrontend::getBoolCtValue(val);

        // Create a CtValue to hold the result
        return Feather_mkCtValueT(node->location, StdDef::typeBool, &res);
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
        if ( !Feather_isCt(arg) )
            REP_ERROR_RET(nullptr, node->location, "ctEval expects an CT argument; %1% given") % arg->type;

        // Make sure we remove all the references
        const Location& loc = arg->location;
        size_t noRefs = arg->type->numReferences;
        for ( size_t i=0; i<noRefs; ++i)
            arg = Feather_mkMemLoad(loc, arg);
        Nest_setContext(arg, node->context);
        if ( !Nest_semanticCheck(arg) )
            return nullptr;

        Node* res = Nest_ctEval(arg);
        if ( res->nodeKind != nkFeatherExpCtValue )
            REP_ERROR_RET(nullptr, arg->location, "Unknown value");

        return res;
    }

    Node* checkAstLift(Node* node)
    {
        Node* arguments = at(node->children, 1);

        // Make sure we have one argument
        if ( Nest_nodeArraySize(arguments->children) != 1 )
            REP_ERROR_RET(nullptr, node->location, "astLift expects 1 argument; %1% given") % Nest_nodeArraySize(arguments->children);

        Node* arg = at(arguments->children, 0);
        // Don't semantically check the argument; let the user of the lift to decide when it's better to do so

        // Create an AST node literal
        return buildLiteral(node->location, fromCStr("CompilerAstNode"), arg);
    }

    Node* checkIfe(Node* node)
    {
        Node* arguments = at(node->children, 1);

        // Make sure we have three arguments
        if ( Nest_nodeArraySize(arguments->children) != 3 )
            REP_ERROR_RET(nullptr, node->location, "ife expects 3 arguments; %1% given") % Nest_nodeArraySize(arguments->children);

        Node* cond = at(arguments->children, 0);
        Node* arg1 = at(arguments->children, 1);
        Node* arg2 = at(arguments->children, 2);
        return mkConditionalExp(node->location, cond, arg1, arg2);
    }

    Node* checkConstruct(Node* node, NodeRange args)
    {
        // Make sure we have more than one argument
        if ( size(args) == 0 )
            REP_ERROR_RET(nullptr, node->location, "construct expects at least 1 argument");

        return createCtorCall(node->location, node->context, args);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Helpers for OperatorCall node
    //

    //! Structure defining the parameters for a search scope
    struct SearchScope {
        StringRef operation;                //!< The operation name to look for
        CompilationContext* searchContext;  //!< The search context
        bool canSearchUp;                   //!< Whether to search up from the given context
        EvalMode mode;                      //!< The evaluation mode to be used
    };

    /**
     * Builds the search scopes for an operator call with the given operation and operands
     *
     * The given 'dest' buffer must contain at least 6 entries. On return
     * 'numScopes' will indicate how many we've filled.
     *
     * We assume that both arguments are expressions.
     *
     * @param dest          [out] Where to place the search scopes
     * @param numScopes     [out] The number of search scopes filled
     * @param node          The node containing the node that initiated the search
     * @param operation     The name of the operation that needs to be called
     * @param opWithPrefix  Prefixed name of operation to be searched (can be empty)
     * @param arg1          The left argument (can be NULL)
     * @param arg2          The right argument (can be NULL)
     * @param searchFromCur True if we are allowed to search from the current node
     */
    void buildSearchScopes(SearchScope* dest, int& numScopes,
            Node* node, StringRef operation, StringRef opWithPrefix, Node* arg1, Node* arg2,
            bool searchFromCur = true) {
        // Identify the first valid operand, so that we can search near it
        Node* base = arg1 ? arg1 : arg2;
        Node* argClass = nullptr;
        if ( base )
        {
            if ( !Nest_semanticCheck(base) )
                return;
            argClass = Feather_classForType(base->type);
        }

        // Gather the search places and parameters

        numScopes = 0;

        EvalMode mode;
        CompilationContext* ctx;

        if ( argClass )
        {
            // Step 1: Try to find an operator that match in the class of the base expression
            mode = base->type->mode;
            ctx = Nest_childrenContext(argClass);
            ASSERT(ctx);
            if ( size(opWithPrefix) > 0 )
                dest[numScopes++] = SearchScope{ opWithPrefix, ctx, false, mode };
            dest[numScopes++] = SearchScope{ operation, ctx, false, mode };

            // Step 2: Try to find an operator that match in the near the class the base expression
            mode = node->context->evalMode;
            ctx = classContext(argClass);
            if ( size(opWithPrefix) > 0 )
                dest[numScopes++] = SearchScope{ opWithPrefix, ctx, false, mode };
            dest[numScopes++] = SearchScope{ operation, ctx, false, mode };
        }

        if ( searchFromCur ) {
            // Step 3: General search from the current context
            mode = node->context->evalMode;
            ctx = node->context;
            if ( size(opWithPrefix) > 0 )
                dest[numScopes++] = SearchScope{ opWithPrefix, ctx, true, mode };
            dest[numScopes++] = SearchScope{ operation, ctx, true, mode };
        }
    }

    //! Performs a scoped search, returning the found declarations
    NodeArray performSearch(SearchScope& ss) {
        ASSERT(ss.searchContext);
        SymTab* sTab = ss.searchContext->currentSymTab;
        NodeArray decls = ss.canSearchUp
                            ? Nest_symTabLookup(sTab, ss.operation.begin)
                            : Nest_symTabLookupCurrent(sTab, ss.operation.begin);
        return decls;
    }


    /**
     * Tries to select an operator call for the given operation and operands.
     *
     * Returns the result of the overloading process.
     * If no definition is found, or if the overloading fails, we return null.
     *
     * @param node         The node containing this operator call
     * @param operation    The name of the operation that needs to be called
     * @param arg1         The left argument (can be NULL)
     * @param arg2         The right argument (can be NULL)
     * @param reportErrors True if we need to report the errors
     *
     * @return The call core required for the selected operation
     */
    Node* selectOperator(Node* node, StringRef operation, Node* arg1, Node* arg2, bool reportErrors)
    {
        Node* argsSrc[] = { arg1, arg2, nullptr };
        NodeRange args = { argsSrc, argsSrc+2 };
        if ( !arg1 ) args.beginPtr++;
        if ( !arg2 ) args.endPtr--;

        // For unary operations, we are also searching with 'pre_', 'post_' prefixes
        string opWithPrefixStr;
        if ( arg1 && !arg2 ) opWithPrefixStr = "post_" + toString(operation);
        if ( !arg1 && arg2 ) opWithPrefixStr = "pre_" + toString(operation);

        // Gather the search places and parameters
        SearchScope searchScopes[6];
        int numScopes = 0;
        buildSearchScopes(searchScopes, numScopes, node, operation, fromString(opWithPrefixStr), arg1, arg2, true);

        // Now actually perform the searchScopes
        OverloadReporting errReporting = reportErrors ? OverloadReporting::noTopLevel : OverloadReporting::none;
        for ( int i=0; i<numScopes; ++i ) {
            SearchScope& s = searchScopes[i];

            // Search for decls in the given context
            NodeArray decls = performSearch(s);

            // Do we have any results? If yes, run the overloading mechanism
            Node* result = nullptr;
            if ( Nest_nodeArraySize(decls) > 0 )
                result = selectOverload(node->context, node->location, s.mode, all(decls), args, errReporting, s.operation);

            // Remove the declarations array
            Nest_freeNodeArray(decls);

            if ( result )
                return result;
        }

        return nullptr;
    }

    Node* checkConvertNullToRefByte(Node* orig)
    {
        if ( Feather_isSameTypeIgnoreMode(orig->type, StdDef::typeNull) )
        {
            Node* res = Feather_mkNull(orig->location, Feather_mkTypeNode(orig->location, StdDef::typeRefByte));
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

        return mkCompoundExp(node->location, arg1, fromString(Nest_toString(arg2)));
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
        arg1Cvt = Feather_mkBitcast(node->location, Feather_mkTypeNode(node->location, StdDef::typeRefByte), arg1Cvt);
        arg2Cvt = Feather_mkBitcast(node->location, Feather_mkTypeNode(node->location, StdDef::typeRefByte), arg2Cvt);

        return Feather_mkFunCall(node->location, StdDef::opRefEq, fromIniList({arg1Cvt, arg2Cvt}));
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
        arg1Cvt = Feather_mkBitcast(node->location, Feather_mkTypeNode(node->location, StdDef::typeRefByte), arg1Cvt);
        arg2Cvt = Feather_mkBitcast(node->location, Feather_mkTypeNode(node->location, StdDef::typeRefByte), arg2Cvt);

        return Feather_mkFunCall(node->location, StdDef::opRefNe, fromIniList({arg1Cvt, arg2Cvt}));
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
        TypeRef arg1BaseType = Feather_removeRef(arg1->type);

        // Check the second type to be null or a reference
        TypeRef arg2Type = arg2->type;
        if ( !Feather_isSameTypeIgnoreMode(arg2Type, StdDef::typeNull) )
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
        return Feather_mkMemStore(node->location, cvt, arg1);
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

    StringRef getOperation(Node* infixExp)
    {
        return Nest_getCheckPropertyString(infixExp, operPropName);
    }

    // Visual explanation:
    //                                                              *
    //     X     <- this        Y     <- keep this pointing here    *
    //    / \                  / \                                  *
    //   Y   R           =>   L   X                                 *
    //  / \                      / \                                *
    // L   M                    M   R                               *
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

        StringRef otherOper = getOperation(other);
        Nest_setPropertyString(other, operPropName, getOperation(node));
        Nest_setPropertyString(node, operPropName, otherOper);
    }

    // Visual explanation:
    //                                                              *
    //     X     <- this        Y     <- keep this pointing here    *
    //    / \                  / \                                  *
    //   L   Y           =>   X   R                                 *
    //      / \              / \                                    *
    //     M   R            L   M                                   *
    //                                                              *
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

        StringRef otherOper = getOperation(other);
        Nest_setPropertyString(other, operPropName, getOperation(node));
        Nest_setPropertyString(node, operPropName, otherOper);
    }

    int getIntValue(Node* node, NodeRange declsOrig, int defaultVal)
    {
        // Expand all the declarations
        NodeArray decls = expandDecls(declsOrig, node);

        // If no declarations found, return the default value
        auto numDecls = size(decls);
        if ( numDecls == 0 )
            return defaultVal;

        // If more than one declarations found, issue a warning
        if ( numDecls > 1 )
        {
            REP_WARNING(node->location, "Multiple precedence declarations found for '%1%'") % getOperation(node);
            for ( Node* decl: decls )
                if ( decl )
                    REP_INFO(decl->location, "See alternative");
            REP_INFO(at(decls, 0)->location, "Using the first alternative");
        }

        // Just one found. Evaluate its value
        Node* n = at(decls, 0);
        if ( !n || !Nest_semanticCheck(n) )
            return defaultVal;
        if ( n->nodeKind == nkSparrowDeclUsing )
            n = at(n->children, 0);
        if ( !n )
            return defaultVal;

        if ( n->type->mode != modeCt )
            REP_INTERNAL(node->location, "Cannot get compile-time integer value from node '%1%'") % Nest_toString(n);

        return getIntCtValue(n);
    }

    int getPrecedence(Node* node)
    {
        Node* arg1 = at(node->children, 0);

        // For prefix operator, search with a special name
        StringRef oper = arg1 ? getOperation(node) : fromCStr("__pre__");

        string precedenceName = "oper_precedence_" + toString(oper);
        string defaultPrecedenceName = "oper_precedence_default";

        // Perform a name lookup for the actual precedence name
        NodeArray decls = Nest_symTabLookup(node->context->currentSymTab, precedenceName.c_str());
        int res = getIntValue(node, all(decls), -1);
        Nest_freeNodeArray(decls);
        if ( res > 0 )
            return res;

        // Search the default precedence name
        decls = Nest_symTabLookup(node->context->currentSymTab, defaultPrecedenceName.c_str());
        res = getIntValue(node, all(decls), -1);
        Nest_freeNodeArray(decls);
        if ( res > 0 )
            return res;

        return 0;
    }

    bool isRightAssociativity(Node* node)
    {
        string assocName = "oper_assoc_" + toString(getOperation(node));

        // Perform a name lookup for the actual associativity name
        NodeArray decls = Nest_symTabLookup(node->context->currentSymTab, assocName.c_str());
        int res = getIntValue(node, all(decls), 1);
        Nest_freeNodeArray(decls);
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

        StringRef curOp = getOperation(node);
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
    StringRef litType = Nest_getCheckPropertyString(node, "spr.literalType");
    StringRef data = Nest_getCheckPropertyString(node, "spr.literalData");

    // Get the type of the literal by looking up the type name
    Node* ident = mkIdentifier(node->location, litType);
    Nest_setContext(ident, node->context);
    if ( !Nest_computeType(ident) )
        return nullptr;
    TypeRef t = getType(ident);
    t = Feather_checkChangeTypeMode(t, modeCt, node->location);

    if ( litType == "StringRef" )
    {
        // Create the explanation
        return Feather_mkCtValueT(node->location, t, &data);
    }
    else
        return Feather_mkCtValue(node->location, t, data);
}

Node* Identifier_SemanticCheck(Node* node)
{
    StringRef id = Nest_getCheckPropertyString(node, "name");

    // Search in the current symbol table for the identifier
    NodeArray declsOrig = Nest_symTabLookup(node->context->currentSymTab, id.begin);
    // If not found, check if we may have an implicit this
    // Try searching for the symbol in the context of the 'this' datatype
    if ( Nest_nodeArraySize(declsOrig) == 0 ) {
        Node* funDecl = Feather_getParentFun(node->context);
        if ( funDecl ) {
            // Does the function has a 'this' param?
            const TypeRef* pThisParamType = Nest_getPropertyType(funDecl, propThisParamType);
            if ( pThisParamType && (*pThisParamType)->hasStorage ) {
                Node* dataType = Feather_classDecl(*pThisParamType);
                if ( dataType ) {
                    declsOrig = Nest_symTabLookupCurrent(dataType->childrenContext->currentSymTab, id.begin);
                }
            }
        }
    }

    if ( Nest_nodeArraySize(declsOrig) == 0 )
        REP_ERROR_RET(nullptr, node->location, "No declarations found with the given name (%1%)") % id;

    // At this point, 'declsOrig' may contain using nodes that will point to other decls
    // Walk over our decls and try to get to the final referred decls
    // Also filter our nodes that cannot be access or without a proper type
    NodeArray decls = expandDecls(all(declsOrig), node);

    if ( size(decls) == 0 ) {
        REP_ERROR(node->location, "No compatible declarations found with the given name (%1%)") % id;

        // Print the removed declarations
        for ( Node* n: declsOrig ) {
            if ( n && n->type )
                REP_INFO(n->location, "See inaccessible declaration");
            else if ( n && n->nodeError )
                REP_INFO(n->location, "See declaration with error");
            else if ( n && !n->type )
                REP_INFO(n->location, "See declaration without a type");
            else
                REP_INFO(node->location, "Encountered invalid declaration");
        }
        Nest_freeNodeArray(declsOrig);
        Nest_freeNodeArray(decls);
        return nullptr;
    }

    // If at least one decl is a field or method, then transform this into a compound expression starting from 'this'
    bool needsThis = false;
    for ( Node* decl: decls )
    {
        ASSERT(decl->type);
        Node* expl = Nest_explanation(decl);
        if ( isField(expl) )
        {
            needsThis = true;
            break;
        }
        if ( funHasImplicitThis(decl) )
        {
            needsThis = true;
            break;
        }
    }
    if ( needsThis )
    {
        // Add 'this' parameter to handle this case
        Node* res = mkCompoundExp(node->location, mkIdentifier(node->location, fromCStr("this")), id);
        return res;
    }

    bool allowDeclExp = 0 != Nest_getPropertyDefaultInt(node, propAllowDeclExp, 0);
    Node* res = getIdentifierResult(node, all(decls), nullptr, allowDeclExp);
    Nest_freeNodeArray(decls);
    Nest_freeNodeArray(declsOrig);
    return res;
}

Node* CompoundExp_SemanticCheck(Node* node)
{
    Node* base = at(node->children, 0);
    StringRef id = Nest_getCheckPropertyString(node, "name");

    // For the base expression allow it to return DeclExp
    Nest_setPropertyExplInt(base, propAllowDeclExp, 1);

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
    Nest_setPropertyNode(node, "baseDataExp", baseDataExp);

    // Get the declarations that this node refers to
    NodeArray declsOrig{0,0,0};
    if ( !baseDecls.empty() )
    {
        // Get the referred declarations; search for our id inside the symbol table of the declarations of the base

        // Get the decls from the first base decl
        if (baseDecls[0]->childrenContext)
            declsOrig = Nest_symTabLookupCurrent(baseDecls[0]->childrenContext->currentSymTab, id.begin);

        // And now from the rest of base decls
        for ( size_t i=1; i<baseDecls.size(); i++ ) {
            Node* baseDecl = baseDecls[i];
            if (!baseDecl->childrenContext)
                continue;
            NodeArray declsCur = Nest_symTabLookupCurrent(baseDecl->childrenContext->currentSymTab, id.begin);
            Nest_appendNodesToArray(&declsOrig, all(declsCur));
            Nest_freeNodeArray(declsCur);
        }
    }
    else if ( base->type->hasStorage )
    {
        // We also consider searching with the 'post_' prefix
        string idWithPrefixStr = "post_" + toString(id);

        // Get the scopes where we search for the id
        SearchScope searchScopes[6];
        int numScopes = 0;
        buildSearchScopes(searchScopes, numScopes, node, id, fromString(idWithPrefixStr), base, nullptr, false);

        // Perform the search sequentially; stop if we find something
        for ( size_t i=0; i<numScopes; i++ ) {
            SearchScope& ss = searchScopes[i];
            declsOrig = performSearch(ss);
            if ( size(declsOrig) > 0 )
                break;
        }
    }

    // At this point, 'declsOrig' may contain using nodes that will point to other decls
    // Walk over our decls and try to get to the final referred decls
    // Also filter our nodes that cannot be access or without a proper type
    NodeArray decls = expandDecls(all(declsOrig), node);

    // Error reporting
    if ( size(decls) == 0 ) {
        // Get the string describing where we are searching
        ostringstream oss;
        if ( baseDecls.empty() ) {
            oss << base << ": " << base->type;
        }
        else if ( baseDecls.size() == 1 ) {
            oss << Feather_getName(baseDecls[0]).begin;
        }
        else {
            oss << "decls(";
            for ( unsigned i=0; i<baseDecls.size(); ++i  ) {
                if ( i>0 ) oss << ", ";
                oss << Feather_getName(baseDecls[i]).begin;
            }
            oss << ")";
        }
        REP_ERROR(node->location, "No declarations found with the name '%1%' inside %2%") % id % oss.str();

        if ( size(declsOrig) == 0 )
            for ( Node* n: baseDecls )
                REP_INFO(n->location, "See searched declaration");

        // Print the removed declarations
        for ( Node* n: declsOrig ) {
            if ( n && n->type )
                REP_INFO(n->location, "See inaccessible declaration");
            else if ( n && n->nodeError )
                REP_INFO(n->location, "See declaration with error");
            else if ( n && !n->type )
                REP_INFO(n->location, "See declaration without a type");
            else
                REP_INFO(node->location, "Encountered invalid declaration");
        }
        Nest_freeNodeArray(declsOrig);
        Nest_freeNodeArray(decls);
        return nullptr;
    }

    bool allowDeclExp = 0 != Nest_getPropertyDefaultInt(node, propAllowDeclExp, 0);
    Node* res = getIdentifierResult(node, all(decls), baseDataExp, allowDeclExp);
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
    ASSERT(base);

    // For the base expression allow it to return DeclExp
    Nest_setPropertyExplInt(base, propAllowDeclExp, 1);

    // Compile the base expression
    // We can expect here both traditional expressions and nodes yielding decl-type types
    if ( !Nest_semanticCheck(base) )
        return nullptr;

    // Check for Sparrow implicit functions
    Node* ident = nullptr;
    if ( base->nodeKind == nkSparrowExpIdentifier )
    {
        ident = base;
        StringRef id = Feather_getName(ident);
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
        else if ( id == "astLift" )
        {
            return checkAstLift(node);
        }
        else if ( id == "ife" )
        {
            return checkIfe(node);
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
        StringRef id = Feather_getName(ident);
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
        else if ( id == "construct" )
        {
            return checkConstruct(node, all(arguments->children));
        }
    }

    // Try to get the declarations pointed by the base node
    Node* thisArg = nullptr;
    NodeVector decls = getDeclsFromNode(base, thisArg);

    string functionName;
    if ( !decls.empty() )
        functionName = toString(Feather_getName(decls.front()));
    else functionName = Nest_toString(base);

    // If we didn't find any declarations, try the operator call
    if ( base->type->hasStorage && decls.empty() )
    {
        Node* cls = Feather_classForType(base->type);
        NodeArray arr = getClassAssociatedDecls(cls, "()");
        decls = toVec(arr);
        Nest_freeNodeArray(arr);
        if ( decls.empty() )
            REP_ERROR_RET(nullptr, node->location, "Class %1% has no user defined call operators") % Feather_getName(cls);
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
        mode = Feather_combineMode(thisArg->type->mode, mode, node->location);
    Node* res = selectOverload(node->context, node->location, mode, all(decls), all(args), OverloadReporting::full, fromString(functionName));

    return res;
}

Node* OperatorCall_SemanticCheck(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 2);
    Node* arg1 = at(node->children, 0);
    Node* arg2 = at(node->children, 1);
    StringRef operation = getOperation(node);

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
    if ( operation == "construct" )
    {
        return checkConstruct(node, all(node->children));
    }

    // Search for the operator
    Node* res = selectOperator(node, operation, arg1, arg2, false);

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
        else if ( size(operation) >= 2 && operation.begin[size(operation)-1] == '=' )
        {
            StringRef baseOperation = operation;
            baseOperation.end--;

            // Transform 'a <op>= b' into 'a = a <op> b'
            op1 = toString(baseOperation);
            op2 = "=";
        }

        if ( !op1.empty() )
        {
            Node* r = selectOperator(node, fromString(op1), a1, a2, false);
            if ( r )
            {
                if ( !Nest_semanticCheck(r) )
                    return nullptr;
                if ( op2 == "!" )
                    res = selectOperator(node, fromString(op2), r, nullptr, false);
                else if ( op2 == "=" )
                    res = selectOperator(node, fromString(op2), a1, r, false);
                else if ( op2.empty() )
                    res = r;
            }
        }
    }

    if ( !res )
    {
        if ( arg1 && arg2 )
            REP_ERROR(node->location, "Cannot find an overload for calling operator %2% %1% %3%") % operation % argTypeStr(arg1) % argTypeStr(arg2);
        else if ( arg1 )
            REP_ERROR(node->location, "Cannot find an overload for calling operator %2% %1%") % operation % argTypeStr(arg1);
        else if ( arg2 )
            REP_ERROR(node->location, "Cannot find an overload for calling operator %1% %2%") % operation % argTypeStr(arg2);
        else
            REP_ERROR(node->location, "Cannot find an overload for calling operator %1%") % operation;

        // Try now once more to select the operator, now with errors turned on
        selectOperator(node, operation, arg1, arg2, true);

        return nullptr;
    }

    return res;
}

Node* InfixExp_SemanticCheck(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 2);

    // This is constructed in such way that left most operations are applied first.
    // This way, we have a tree that has a lot of children on the left side and one children on the right side
    // When applying the precedence and associativity rules for the expressions in here, we will try to balance this
    // tree, and move some elements to the right side of the tree by performing some swaps

    handlePrecedence(node);
    handleAssociativity(node);

    Node* arg1 = at(node->children, 0);
    Node* arg2 = at(node->children, 1);

    return mkOperatorCall(node->location, arg1, getOperation(node), arg2);
}

Node* LambdaFunction_SemanticCheck(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->referredNodes) == 4);
    Node* parameters = at(node->referredNodes, 0);
    Node* returnType = at(node->referredNodes, 1);
    Node* body = at(node->referredNodes, 2);
    Node* closureParams = at(node->referredNodes, 3);

    Location loc = node->location;

    // Create the enclosing class body node list; add here variables corresponding to the closure params
    Node* classBody = Feather_mkNodeList(loc, {});
    if ( closureParams )
    {
        for ( Node* arg: closureParams->children )
        {
            // Get the name and the type of the argument
            if ( !arg )
                REP_INTERNAL(closureParams->location, "Invalid closure parameter");
            ASSERT(arg);
            if ( arg->nodeKind != nkSparrowExpIdentifier )
                REP_INTERNAL(arg->location, "The closure parameter must be an identifier");
            Nest_setContext(arg, node->context);
            if ( !Nest_semanticCheck(arg) )
                return nullptr;
            StringRef varName = Feather_getName(arg);
            TypeRef varType = Feather_removeLValueIfPresent(arg->type);

            // Create a similar variable in the enclosing class - must have the same name
            const Location& argLoc = arg->location;
            Nest_appendNodeToArray(&classBody->children, mkSprVariable(argLoc, varName, varType, nullptr));
        }
    }

    // Create the lambda closure class
    Node* closureClass = mkSprClass(loc, fromCStr("$lambdaEnclosureData"), nullptr, nullptr, nullptr, classBody);
    if ( closureParams && size(closureParams->children) > 0 )
        Nest_setPropertyInt(closureClass, propGenerateInitCtor, 1);

    // The actual enclosed function -- ensure adding a 'this' parameter
    Node* thisParamTypeNode = mkOperatorCall(loc, nullptr, fromCStr("@"), mkIdentifier(loc, fromCStr("$lambdaEnclosureData")));
    Node* thisParam = mkSprParameter(loc, fromCStr("this"), thisParamTypeNode);
    Node* parametersWithThis = Feather_mkNodeList(loc, fromIniList({thisParam}));
    Feather_appendNodeList(parametersWithThis, parameters);
    Node* enclosedFun = mkSprFunction(loc, fromCStr("()"), parametersWithThis, returnType, body);
    Nest_setPropertyInt(enclosedFun, propIsStatic, 1);

    // Create the enclosing package node list
    Node* packageBody = Feather_mkNodeList(loc, fromIniList({closureClass, enclosedFun}));
    Node* closurePackage = mkSprPackage(loc, fromCStr("$lambdaEnclosurePackage"), packageBody);


    // Add the closure package as a top level node of this node
    // We add it in the context of the parent function
    Node* parentFun = Feather_getParentFun(node->context);
    CompilationContext* parentContext = parentFun ? parentFun->context : node->context;
    Nest_setContext(closurePackage, parentContext);
    Nest_appendNodeToArray(&node->additionalNodes, closurePackage);

    // Compute the type & semantically check the resulting package
    if ( !Nest_computeType(closurePackage) )
        return nullptr;
    if ( !Nest_semanticCheck(closurePackage) )
        return nullptr;

    // Create a resulting object: a constructor call to our class
    Node* cls = Nest_explanation(closureClass);
    ASSERT(cls);
    Node* classId = createTypeNode(node->context, loc, Feather_getDataType(cls, 0, modeRtCt));
    return mkFunApplication(loc, classId, closureParams);
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

    return Feather_mkConditional(node->location, cond, alt1, alt2);
}

Node* DeclExp_SemanticCheck(Node* node)
{
    // Make sure we computed the type for all the referred nodes
    for ( Node* n: node->referredNodes )
    {
        if ( n && !Nest_computeType(n) )
            continue;
    }
    node->type = Feather_getVoidType(node->context->evalMode);
    return node;    // This node should never be translated directly
}

Node* StarExp_SemanticCheck(Node* node)
{
    Node* base = at(node->children, 0);

    // For the base expression allow it to return DeclExp
    Nest_setPropertyExplInt(base, propAllowDeclExp, 1);

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
        auto newEntries = toVec(Nest_symTabAllEntries(baseSymTab));
        decls.insert(decls.begin(), newEntries.begin(), newEntries.end());
    }

    // Make sure the declarations are unique
    sort(decls.begin(), decls.end());
    decls.erase(unique(decls.begin(), decls.end()), decls.end());

    // Remove all the inaccessible entries
    for ( Node*& decl : decls ) {
        if ( !canAccessNode(decl, node) )
            decl = nullptr;
    }
    auto it = remove_if(decls.begin(), decls.end(), [](Node* d) { return d==nullptr; });
    decls.erase(it, decls.end());

    // This expands to a declaration expression
    return mkDeclExp(node->location, all(decls));
}

Node* ModuleRef_SemanticCheck(Node* node)
{
    Node* module = at(node->referredNodes, 0);
    if ( !module )
        return nullptr;

    if ( module->nodeKind == nkSparrowDeclModule ) {
        // If we are referring a Sparrow module, point to the inner most package
        Node* innerMostPackage = Nest_getCheckPropertyNode(module, propResultingDecl);
        Nest_setPropertyNode(node, propResultingDecl, innerMostPackage);
    }
    else {
        // Point to the main node; assume it's a package or something
        Nest_setPropertyNode(node, propResultingDecl, module);
    }

    node->type = Feather_getVoidType(node->context->evalMode);
    return node;    // This node should never be translated directly
}

