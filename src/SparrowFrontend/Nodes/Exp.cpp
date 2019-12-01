#include "SparrowFrontend/StdInc.h"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Stmt.hpp"
#include "SparrowFrontend/Nodes/Builder.h"
#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "SparrowFrontend/Helpers/Ct.h"
#include "SparrowFrontend/Helpers/CommonCode.h"
#include "SparrowFrontend/Services/IConvertService.h"
#include "SparrowFrontend/Services/IOverloadService.h"

#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/cppif/NodeHelpers.hpp"

namespace SprFrontend {

namespace {
using Feather::DeclNode;

////////////////////////////////////////////////////////////////////////////
// Helpers for Identifier and CompoundExp nodes
//

bool isField(NodeHandle node) {
    if (node.kind() != nkFeatherDeclVar)
        return false;
    return nullptr != Feather_getParentClass(node.context());
}

//! Combine type categories for field refs; the inputs and output are type kinds
int combineCatForFieldRef(int baseCat, int fieldCat) {
    if (fieldCat == typeKindConst)
        return typeKindConst;
    else if (fieldCat == typeKindMutable)
        return typeKindMutable;
    else
        return baseCat;
}

//! Check for special case in which we should ignore a const in field-ref:
//! Whenever we are in a ctor, and the field-ref is based of 'this'
bool shouldIgnoreConst(NodeHandle baseExp) {
    // Check that the base expression is 'this' identifier
    Identifier baseId = baseExp.kindCast<Identifier>();
    if (!baseId || baseId.name() != "this")
        return false;

    // Check that we are in a function named 'ctor' or 'dtor'
    Feather::DeclNode fun = Feather_getParentFun(baseId.context());
    if (!fun || (fun.name() != "ctor" && fun.name() != "dtor"))
        return false;

    return true;
}

//! Create a field ref expression following Sparrow frontend rules
Feather::FieldRefExp createFieldRef(const Location& loc, NodeHandle baseExp, Feather::VarDecl var) {
    // First, the base expression needs to be valid
    if (!baseExp)
        REP_INTERNAL(loc, "No base expression to refer to a field");
    ASSERT(baseExp);

    // Make sure the base is a cat-type
    // In the generated ctorFromCt, the bound 'other' value is without any cat type
    if (baseExp.type().numReferences() == 0) {
        auto destType = Feather::ConstType::get(TypeWithStorage(baseExp.type()));
        auto ctx = baseExp.context();
        baseExp = createTmpForRef(baseExp, destType);
        baseExp.setContext(ctx);
        if (!baseExp.computeType())
            return {};
    }
    auto origBaseExp = baseExp;

    Type t = baseExp.type();
    if (!t.hasStorage())
        REP_INTERNAL(loc, "Invalid type for base of field expression: %1%") % baseExp;
    assert(t.hasStorage());

    int origTypeCat = t.kind();

    // Dereference to have just a cat type to the field
    if (t.numReferences() > 1) {
        // If we have N references apply N-1 dereferencing operations
        for (size_t i = 1; i < t.numReferences(); ++i) {
            baseExp = Feather::MemLoadExp::create(loc, baseExp);
        }
        t = Feather::DataType::get(t.referredNode(), t.mode()); // Zero references
    }

    // Compute the desired category of the base expression type
    int desiredTypeCat = origTypeCat;
    const int* fieldCat = var.getPropertyInt(propSprOrigCat);
    if (fieldCat) {
        desiredTypeCat = combineCatForFieldRef(origTypeCat, *fieldCat);
    }

    // If the desired category is const, but we are initializing a filed in the ctor, transform it
    // into mutable
    if (desiredTypeCat == typeKindConst && origTypeCat != typeKindConst) {
        if (shouldIgnoreConst(origBaseExp))
            desiredTypeCat = origTypeCat;
    }

    // Do we need to change the type of the base expression?
    if (desiredTypeCat != origTypeCat) {
        t = Feather::changeCat(TypeWithStorage(t), desiredTypeCat);
        auto typeNode = Feather::TypeNode::create(loc, t);
        baseExp = Feather::BitcastExp::create(loc, typeNode, baseExp);
    }

    return Feather::FieldRefExp::create(loc, baseExp, var);
}

NodeHandle getIdentifierResult(
        NodeHandle node, NodeRange decls, NodeHandle baseExp, bool allowDeclExp) {
    const Location& loc = node.location();

    // If this points to one declaration only, try to use that declaration
    if (decls.size() == 1) {
        NodeHandle resDecl = resultingDecl(decls[0]);
        ASSERT(resDecl);

        // Check if we can refer to a variable
        Feather::VarDecl resDeclVar = resDecl.kindCast<Feather::VarDecl>();
        if (resDeclVar) {
            if (isField(resDecl))
                return createFieldRef(loc, baseExp, resDeclVar);
            else
                return Feather::VarRefExp::create(loc, resDeclVar);
        }

        // If this is a using, get its value
        UsingDecl baseExpUsing = resDecl.kindCast<UsingDecl>();
        if (baseExpUsing)
            return baseExpUsing.usedNode();

        // If we allow decl expressions, create a decl expression out of this
        if (allowDeclExp)
            return DeclExp::create(loc, decls, baseExp);

        // Try to convert this to a type
        Type t;
        Feather::StructDecl resDeclStruct = resDecl.kindCast<Feather::StructDecl>();
        if (resDeclStruct) {
            t = Feather::DataType::get(resDeclStruct, resDeclStruct.effectiveMode());
        }
        if (resDecl.kind() == nkSparrowDeclSprConcept ||
                resDecl.kind() == nkSparrowDeclGenericDatatype)
            t = ConceptType::get(resDecl);
        if (t)
            return createTypeNode(node.context(), loc, t);
    }

    // If we allow decl expressions, create a decl expression out of this
    if (allowDeclExp)
        return DeclExp::create(loc, decls, baseExp);

    // If we are here, this identifier could only represent a function application
    auto fapp = FunApplication::create(loc, DeclExp::create(loc, decls, baseExp), nullptr);
    fapp.setContext(node.context());
    return fapp.semanticCheck();
}

////////////////////////////////////////////////////////////////////////////
// Helpers for FunApplication node
//

bool checkCastArguments(const Location& loc, const char* castName, NodeRange arguments,
        bool secondShouldBeRef = false) {
    // Make sure we have only 2 arguments
    auto numArgs = arguments.size();
    if (numArgs != 2) {
        REP_ERROR(loc, "%1% expects 2 arguments; %2% given") % castName % numArgs;
        return false;
    }
    auto arg1 = arguments[0];
    auto arg2 = arguments[1];

    if (!arg1.type() || !arg2.type())
        REP_INTERNAL(loc, "Invalid arguments");
    ASSERT(arg1.type() && arg2.type());

    // Make sure the first argument is a type
    Type t = getType(arg1);
    if (!t) {
        REP_ERROR(arg1.location(), "The first argument of a %1% must be a type") % castName;
        return false;
    }

    // Make sure that the second argument has storage
    if (!arg2.type().hasStorage()) {
        REP_ERROR(arg2.location(),
                "The second argument of a %1% must have a storage type (we have %2%)") %
                castName % arg2.type();
        return false;
    }

    if (secondShouldBeRef && arg2.type().numReferences() == 0) {
        REP_ERROR(
                arg2.location(), "The second argument of a %1% must be a reference (we have %2%)") %
                castName % arg2.type();
        return false;
    }
    return true;
}

NodeHandle checkStaticCast(NodeHandle node) {
    NodeHandle arguments = node.children()[1];

    if (!arguments)
        REP_ERROR_RET(nullptr, node.location(), "No arguments given to cast");

    // Check arguments
    if (!checkCastArguments(node.location(), "cast", arguments.children()))
        return nullptr;

    auto destTypeArg = arguments.children()[0];
    auto srcArg = arguments.children()[1];

    Type destType = getType(destTypeArg);
    Type srcType = srcArg.type();

    // Check if we can cast
    ConversionResult c = g_ConvertService->checkConversion(srcArg, destType);
    if (!c)
        REP_ERROR_RET(
                nullptr, node.location(), "Cannot cast from %1% to %2%; types are unrelated") %
                srcType % destType;
    NodeHandle result = c.apply(srcArg);
    result.handle->location = node.location();

    return result;
}

NodeHandle checkReinterpretCast(NodeHandle node) {
    NodeHandle arguments = node.children()[1];

    if (!arguments)
        REP_ERROR_RET(nullptr, node.location(), "No arguments given to reinterpretCast");

    // Check arguments
    if (!checkCastArguments(node.location(), "reinterpretCast", arguments.children(), true))
        return nullptr;

    auto destTypeArg = arguments.children()[0];
    auto srcArg = arguments.children()[1];

    Type destType = getType(destTypeArg);
    Type srcType = srcArg.type();

    ASSERT(destType);
    ASSERT(destType.hasStorage());
    if (destType.numReferences() == 0)
        REP_ERROR_RET(nullptr, destTypeArg.location(),
                "Destination type must be a reference (currently: %1%)") %
                destType;

    // If source is a category type and the number of source reference is greater than the
    // destination references, remove category
    if (srcType.numReferences() > destType.numReferences() && Feather::isCategoryType(srcType))
        srcArg = Feather::MemLoadExp::create(srcArg.location(), srcArg);

    // Generate a bitcast operation out of this node
    return Feather::BitcastExp::create(
            node.location(), Feather::TypeNode::create(destTypeArg.location(), destType), srcArg);
}

NodeHandle checkSizeOf(NodeHandle node) {
    NodeHandle arguments = node.children()[1];

    if (!arguments)
        REP_ERROR_RET(nullptr, node.location(), "No arguments given to sizeOf");

    // Make sure we have only one argument
    if (arguments.children().size() != 1)
        REP_ERROR_RET(nullptr, node.location(), "sizeOf expects one argument; %1% given") %
                arguments.children().size();
    NodeHandle arg = arguments.children()[0];
    Type t = arg.type();
    if (!t)
        REP_INTERNAL(node.location(), "Invalid argument");

    // Make sure that the argument has storage, or it represents a type
    t = evalTypeIfPossible(arg);
    if (!t.hasStorage()) {
        REP_ERROR_RET(nullptr, arg.location(),
                "The argument of sizeOf must be a type or an expression with storage type (we have "
                "%1%)") %
                arg.type();
    }

    // Make sure the class that this refers to has the type properly computed
    NodeHandle cls = t.referredNode();
    NodeHandle mainNode = cls.childrenContext()->currentSymTab->node;
    if (!mainNode.computeType())
        return nullptr;

    // Remove category if we have some
    t = Feather::removeCategoryIfPresent(t);

    // Get the size of the type of the argument
    int size = Nest_sizeOf(t);

    // Create a CtValue to hold the size
    return Feather_mkCtValueT(node.location(), StdDef::typeInt, &size);
}

NodeHandle checkTypeOf(NodeHandle node) {
    NodeHandle arguments = node.children()[1];

    if (!arguments)
        REP_ERROR_RET(nullptr, node.location(), "No arguments given to typeOf");
    if (arguments.children().size() != 1)
        REP_ERROR_RET(nullptr, node.location(), "typeOf expects one argument; %1% given") %
                arguments.children().size();
    NodeHandle arg = arguments.children()[0];

    // Compile the argument
    if (!arguments.semanticCheck())
        return nullptr;

    // Make sure we have only one argument
    Type t = arg.type();
    if (!t)
        REP_INTERNAL(node.location(), "Invalid argument");
    t = Feather::removeCategoryIfPresent(t);

    // Create a CtValue to hold the type
    return createTypeNode(node.context(), arg.location(), t);
}

bool checkIsValidImpl(const Location& loc, NodeHandle arguments, const char* funName = "isValid",
        int numArgs = 1) {
    if (!arguments) {
        REP_ERROR(loc, "No arguments given to %1") % funName;
        return false;
    }

    // Make sure we have only one argument
    if ((int)arguments.children().size() != numArgs) {
        REP_ERROR(loc, "%1% expects %2% arguments; %3% given") % funName % numArgs %
                arguments.children().size();
        return false;
    }

    // Try to compile the argument
    bool isValid = false;
    int oldVal = Nest_isReportingEnabled();
    Nest_enableReporting(0);
    NodeHandle res = arguments.semanticCheck();
    isValid = res && !arguments.hasError() && !arguments.children()[0].hasError() &&
              Nest_getSuppressedErrorsNum() == 0;
    Nest_enableReporting(oldVal);
    return isValid;
}

NodeHandle checkIsValid(FunApplication node) {
    NodeList arguments = node.arguments();

    bool isValid = checkIsValidImpl(node.location(), arguments, "isValid");

    // Create a CtValue to hold the result
    return Feather_mkCtValueT(node.location(), StdDef::typeBool, &isValid);
}

NodeHandle checkIsValidAndTrue(FunApplication node) {
    NodeList arguments = node.arguments();

    bool res = checkIsValidImpl(node.location(), arguments, "isValidAndTrue");
    NodeHandle arg;
    if (res) {
        arg = arguments.children()[0];
        if (!arg.semanticCheck())
            res = false;
    }

    // The expression must be CT
    if (res && !Feather_isCt(arg)) {
        REP_ERROR(node.location(), "ctEval expects an CT argument; %1% given") % arg.type();
        res = false;
    }

    if (res) {
        // Make sure we remove all the references
        const Location& loc = arg.location();
        size_t noRefs = arg.type().numReferences();
        for (size_t i = 0; i < noRefs; ++i)
            arg = Feather::MemLoadExp::create(loc, arg);
        arg.setContext(node.context());
        if (!arg.semanticCheck())
            res = false;
    }

    NodeHandle val = nullptr;
    if (res) {
        val = Nest_ctEval(arg);
        if (val.kind() != nkFeatherExpCtValue) {
            REP_ERROR(arg.location(), "Unknown value");
            res = false;
        }
    }

    // Get the value from the CtValue object
    if (res)
        res = SprFrontend::getBoolCtValue(val);

    // Create a CtValue to hold the result
    return Feather_mkCtValueT(node.location(), StdDef::typeBool, &res);
}

NodeHandle checkValueIfValid(FunApplication node) {
    NodeList arguments = node.arguments();

    bool isValid = checkIsValidImpl(node.location(), arguments, "valueIfValid", 2);

    return isValid ? arguments.children()[0] : arguments.children()[1];
}

NodeHandle checkCtEval(FunApplication node) {
    NodeList arguments = node.arguments();

    // Make sure we have one argument
    if (arguments.children().size() != 1)
        REP_ERROR_RET(nullptr, node.location(), "ctEval expects 1 argument; %1% given") %
                arguments.children().size();

    NodeHandle arg = arguments.children()[0];
    if (!arg.semanticCheck())
        return nullptr;

    // The expression must be CT
    if (!Feather_isCt(arg))
        REP_ERROR_RET(nullptr, node.location(), "ctEval expects an CT argument; %1% given") %
                arg.type();

    // Make sure we remove all the references
    const Location& loc = arg.location();
    size_t noRefs = arg.type().numReferences();
    for (size_t i = 0; i < noRefs; ++i)
        arg = Feather::MemLoadExp::create(loc, arg);
    arg.setContext(node.context());
    if (!arg.semanticCheck())
        return nullptr;

    NodeHandle res = Nest_ctEval(arg);
    if (res.kind() != nkFeatherExpCtValue)
        REP_ERROR_RET(nullptr, arg.location(), "Unknown value");

    return res;
}

NodeHandle checkAstLift(FunApplication node) {
    NodeList arguments = node.arguments();

    // Make sure we have one argument
    if (arguments.children().size() != 1)
        REP_ERROR_RET(nullptr, node.location(), "astLift expects 1 argument; %1% given") %
                arguments.children().size();

    NodeHandle arg = arguments.children()[0];
    // Don't semantically check the argument; let the user of the lift to decide when it's better to
    // do so

    // Create an AST node literal
    return buildLiteral(node.location(), StringRef("CompilerAstNode"), arg);
}

NodeHandle checkIfe(FunApplication node) {
    NodeList arguments = node.arguments();

    // Make sure we have three arguments
    if (arguments.children().size() != 3)
        REP_ERROR_RET(nullptr, node.location(), "ife expects 3 arguments; %1% given") %
                arguments.children().size();

    NodeHandle cond = arguments.children()[0];
    NodeHandle arg1 = arguments.children()[1];
    NodeHandle arg2 = arguments.children()[2];
    return ConditionalExp::create(node.location(), cond, arg1, arg2);
}

NodeHandle checkConstruct(NodeHandle node, NodeRange args) {
    // Make sure we have more than one argument
    if (args.size() == 0)
        REP_ERROR_RET(nullptr, node.location(), "construct expects at least 1 argument");

    return createCtorCall(node.location(), node.context(), args);
}

////////////////////////////////////////////////////////////////////////////
// Helpers for OperatorCall node
//

//! Structure defining the parameters for a search scope
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct SearchScope {
    StringRef operation;               //!< The operation name to look for
    CompilationContext* searchContext; //!< The search context
    bool canSearchUp;                  //!< Whether to search up from the given context
    EvalMode mode;                     //!< The evaluation mode to be used
};

//! The maximum number of search scopes
constexpr int maxNumSearchScopes = 6;

//! Structure describing all the possible search scopes for a given name search
struct SearchScopes {
    SearchScope scopes[maxNumSearchScopes]; //!< The search scopes
    int numScopes{0};                       //!< The number of search scopes we are using

    void addScope(const SearchScope& scope) {
        ASSERT(numScopes < maxNumSearchScopes);
        scopes[numScopes++] = scope;
    }
};

const SearchScope* begin(const SearchScopes& ss) { return &ss.scopes[0]; }
const SearchScope* end(const SearchScopes& ss) { return &ss.scopes[ss.numScopes]; }

enum class SearchType {
    complete,      //!< Inside dataytype, near datatype and from current node
    insideAndNear, //!< Inside dataytype and near datatype
    nearOnly,      //!< Near datatype only
};

/**
 * Builds the search scopes for an operator call with the given operation and operands
 *
 * The given 'dest' buffer must contain at least 6 entries. On return
 * 'numScopes' will indicate how many we've filled.
 *
 * We assume that both arguments are expressions.
 *
 * @param nodeCtx       The context of the node from which we initiated the search
 * @param operation     The name of the operation that needs to be called
 * @param opWithPrefix  Prefixed name of operation to be searched (can be empty)
 * @param base          The base arguments that can be used to search from (can be NULL)
 * @param searchType    Indicates the possible options for search scopes
 *
 * @return The search scopes corresponding to the given parameters
 */
SearchScopes buildSearchScopes(CompilationContext* nodeCtx, StringRef operation,
        StringRef opWithPrefix, NodeHandle base, SearchType searchType) {
    SearchScopes res;

    // Identify the first valid operand, so that we can search near it
    Feather::DeclNode baseDecl;
    if (base) {
        if (!base.semanticCheck())
            return res;
        baseDecl = DeclNode(base.type().referredNode());
    }

    // Gather the search places and parameters
    EvalMode mode;
    CompilationContext* ctx = nullptr;
    bool searchInside = baseDecl && searchType != SearchType::nearOnly;
    bool searchNear = baseDecl;
    bool searchFromCur = searchType == SearchType::complete;

    // Step 1: Try to find an operator that match in the class of the base expression
    if (searchInside) {
        ASSERT(base);
        mode = base.type().mode();
        ctx = baseDecl.childrenContext();
        ASSERT(ctx);
        if (opWithPrefix)
            res.addScope(SearchScope{opWithPrefix, ctx, false, mode});
        res.addScope(SearchScope{operation, ctx, false, mode});
    }

    // Step 2: Try to find an operator that match in the near the class the base expression
    if (searchNear) {
        mode = nodeCtx->evalMode;
        ctx = classContext(baseDecl);
        if (opWithPrefix)
            res.addScope(SearchScope{opWithPrefix, ctx, false, mode});
        res.addScope(SearchScope{operation, ctx, false, mode});
    }

    // Step 3: General search from the current context
    if (searchFromCur) {
        mode = nodeCtx->evalMode;
        ctx = nodeCtx;
        if (opWithPrefix)
            res.addScope(SearchScope{opWithPrefix, ctx, true, mode});
        res.addScope(SearchScope{operation, ctx, true, mode});
    }

    return res;
}

//! Performs a scoped search, returning the found declarations
Nest_NodeArray performSearch(const SearchScope& ss) {
    ASSERT(ss.searchContext);
    Nest_SymTab* sTab = ss.searchContext->currentSymTab;
    auto decls = ss.canSearchUp ? Nest_symTabLookup(sTab, ss.operation.begin)
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
 * @return The call code required for the selected operation
 */
NodeHandle selectOperator(OperatorCall node, StringRef operation, NodeHandle arg1, NodeHandle arg2,
        bool reportErrors) {
    Node* argsSrc[] = {arg1, arg2};
    Nest_NodeRange args = {argsSrc, argsSrc + 2};
    if (!arg1)
        args.beginPtr++;
    if (!arg2)
        args.endPtr--;

    // For unary operations, we are also searching with 'pre_', 'post_' prefixes
    string opWithPrefixStr;
    if (arg1 && !arg2)
        opWithPrefixStr = "post_" + operation.toStd();
    if (!arg1 && arg2)
        opWithPrefixStr = "pre_" + operation.toStd();

    // Gather the search places and parameters
    NodeHandle base = arg1 ? arg1 : arg2;
    SearchScopes searchScopes = buildSearchScopes(
            node.context(), operation, StringRef(opWithPrefixStr), base, SearchType::complete);

    // Now actually perform the searchScopes
    OverloadReporting errReporting =
            reportErrors ? OverloadReporting::noTopLevel : OverloadReporting::none;
    for (const SearchScope& s : searchScopes) {
        // Search for decls in the given context
        auto decls = performSearch(s);

        // Do we have any results? If yes, run the overloading mechanism
        NodeHandle result;
        if (size(decls) > 0)
            result = g_OverloadService->selectOverload(node.context(), node.location(), s.mode,
                    all(decls), args, errReporting, s.operation);
        // NOTE: If we found names, but overloading fails, we continue searching other scopes

        // Remove the declarations array
        Nest_freeNodeArray(decls);

        if (result)
            return result;
    }

    return {};
}

NodeHandle checkConvertNullToRefByte(NodeHandle orig) {
    if (Nest::sameTypeIgnoreMode(orig.type(), StdDef::typeNull)) {
        auto res = Feather::NullExp::create(
                orig.location(), Feather::TypeNode::create(orig.location(), StdDef::typeRefByte));
        res.setContext(orig.context());
        if (!res.computeType())
            REP_INTERNAL(orig.location(), "Cannot compute type to NullExp node");
        return res;
    }
    return orig;
}

NodeHandle handleRefEq(OperatorCall node) {
    auto arg1 = node.arg1();
    auto arg2 = node.arg2();

    if (!arg1.semanticCheck() || !arg2.semanticCheck())
        return {};

    // If we have null as arguments, convert them to "RefByte"
    arg1 = checkConvertNullToRefByte(arg1);
    arg2 = checkConvertNullToRefByte(arg2);

    // Make sure that both the arguments are references
    if (arg1.type().numReferences() == 0)
        REP_ERROR_RET(nullptr, node.location(),
                "Left operand of a reference equality operator is not a reference (%1%)") %
                arg1.type();
    if (arg2.type().numReferences() == 0)
        REP_ERROR_RET(nullptr, node.location(),
                "Right operand of a reference equality operator is not a reference (%1%)") %
                arg2.type();

    NodeHandle arg1Cvt;
    NodeHandle arg2Cvt;
    doDereference1(arg1, arg1Cvt); // Dereference until the last reference
    doDereference1(arg2, arg2Cvt);
    arg1Cvt = Feather::BitcastExp::create(node.location(),
            Feather::TypeNode::create(node.location(), StdDef::typeRefByte), arg1Cvt);
    arg2Cvt = Feather::BitcastExp::create(node.location(),
            Feather::TypeNode::create(node.location(), StdDef::typeRefByte), arg2Cvt);

    return Feather::FunCallExp::create(
            node.location(), StdDef::opRefEq, fromIniList({arg1Cvt, arg2Cvt}));
}

NodeHandle handleRefNe(OperatorCall node) {
    auto arg1 = node.arg1();
    auto arg2 = node.arg2();

    if (!arg1.semanticCheck() || !arg2.semanticCheck())
        return {};

    // If we have null as arguments, convert them to "RefByte"
    arg1 = checkConvertNullToRefByte(arg1);
    arg2 = checkConvertNullToRefByte(arg2);

    // Make sure that both the arguments are references
    if (arg1.type().numReferences() == 0)
        REP_ERROR_RET(nullptr, node.location(),
                "Left operand of a reference equality operator is not a reference (%1%)") %
                arg1.type();
    if (arg2.type().numReferences() == 0)
        REP_ERROR_RET(nullptr, node.location(),
                "Right operand of a reference equality operator is not a reference (%1%)") %
                arg2.type();

    NodeHandle arg1Cvt;
    NodeHandle arg2Cvt;
    doDereference1(arg1, arg1Cvt); // Dereference until the last reference
    doDereference1(arg2, arg2Cvt);
    arg1Cvt = Feather::BitcastExp::create(node.location(),
            Feather::TypeNode::create(node.location(), StdDef::typeRefByte), arg1Cvt);
    arg2Cvt = Feather::BitcastExp::create(node.location(),
            Feather::TypeNode::create(node.location(), StdDef::typeRefByte), arg2Cvt);

    return Feather::FunCallExp::create(
            node.location(), StdDef::opRefNe, fromIniList({arg1Cvt, arg2Cvt}));
}

NodeHandle handleRefAssign(OperatorCall node) {
    auto arg1 = node.arg1();
    auto arg2 = node.arg2();

    if (!arg1.semanticCheck() || !arg2.semanticCheck())
        return {};

    // Make sure the first argument is a reference reference
    if (arg1.type().numReferences() < 2)
        REP_ERROR_RET(nullptr, node.location(),
                "Left operand of a reference assign operator is not a reference reference (%1%)") %
                arg1.type();
    Type arg1BaseType = Feather::dereferenceType(TypeWithStorage(arg1.type()));

    // Check the second type to be null or a reference
    Type arg2Type = arg2.type();
    if (!Nest::sameTypeIgnoreMode(arg2Type, StdDef::typeNull)) {
        if (arg2Type.numReferences() == 0)
            REP_ERROR_RET(nullptr, node.location(),
                    "Right operand of a reference assign operator is not a reference (%1%)") %
                    arg2.type();
    }

    // Check for a conversion from the second argument to the first argument
    ConversionResult c = g_ConvertService->checkConversion(arg2, arg1BaseType);
    if (!c)
        REP_ERROR_RET(nullptr, node.location(), "Cannot convert from %1% to %2%") % arg2.type() %
                arg1BaseType;
    NodeHandle cvt = c.apply(arg2);

    // Return a memstore operator
    return Feather::MemStoreExp::create(node.location(), cvt, arg1);
}

NodeHandle handleFunPtr(OperatorCall node) { return createFunPtr(node.arg2()); }

const char* argTypeStr(NodeHandle node) {
    return node && node.type() ? node.type().description() : "?";
}

//! Helper function that searches for names from a datatype (inside and/or near)
Nest_NodeArray searchDeclsFromDatatype(CompilationContext* nodeCtx, NodeHandle base, StringRef id,
        StringRef idPrefix, SearchType searchType = SearchType::insideAndNear) {

    Nest_NodeArray decls{nullptr, nullptr, nullptr};

    // Get the scopes where we search for the id
    SearchScopes searchScopes = buildSearchScopes(nodeCtx, id, idPrefix, base, searchType);

    // Perform the search sequentially; stop if we find something
    for (const SearchScope& s : searchScopes) {
        decls = performSearch(s);
        if (size(decls) > 0)
            break;
    }

    return decls;
}

////////////////////////////////////////////////////////////////////////////
// Helpers for InfixExp node
//

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
void swapLeft(InfixOp node) {
    InfixOp other = InfixOp(node.arg1());

    node.childrenM()[0] = other.children()[0];
    other.childrenM()[0] = other.children()[1];
    other.childrenM()[1] = node.children()[1];
    node.childrenM()[1] = other;

    StringRef otherOper = other.oper();
    other.setProperty(propSprOperation, node.oper());
    node.setProperty(propSprOperation, otherOper);
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
void swapRight(InfixOp node) {
    InfixOp other = InfixOp(node.arg2());

    node.childrenM()[1] = other.children()[1];
    other.childrenM()[1] = other.children()[0];
    other.childrenM()[0] = node.children()[0];
    node.childrenM()[0] = other;

    StringRef otherOper = other.oper();
    other.setProperty(propSprOperation, node.oper());
    node.setProperty(propSprOperation, otherOper);
}

int getIntValue(InfixOp node, NodeRange declsOrig, int defaultVal) {
    // Expand all the declarations
    auto decls = expandDecls(declsOrig, node);

    // If no declarations found, return the default value
    auto numDecls = size(decls);
    if (numDecls == 0)
        return defaultVal;

    // If more than one declarations found, issue a warning
    if (numDecls > 1) {
        REP_WARNING(node.location(), "Multiple precedence declarations found for '%1%'") %
                node.oper();
        for (Node* decl : decls)
            if (decl)
                REP_INFO(decl->location, "See alternative");
        REP_INFO(at(decls, 0)->location, "Using the first alternative");
    }

    // Just one found. Evaluate its value
    NodeHandle n = at(decls, 0);
    if (!n || !n.semanticCheck())
        return defaultVal;
    if (n.kind() == nkSparrowDeclUsing)
        n = n.children()[0];
    if (!n)
        return defaultVal;

    if (n.type().mode() != modeCt)
        REP_INTERNAL(node.location(), "Cannot get compile-time integer value from node '%1%'") % n;

    return getIntCtValue(n);
}

int getPrecedence(InfixOp node) {
    NodeHandle arg1 = node.arg1();

    // For prefix operator, search with a special name
    StringRef oper = arg1 ? node.oper() : StringRef("__pre__");

    string precedenceName = "oper_precedence_" + StringRef(oper).toStd();
    auto defaultPrecedenceName = "oper_precedence_default";

    // Perform a name lookup for the actual precedence name
    auto decls = Nest_symTabLookup(node.context()->currentSymTab, precedenceName.c_str());
    int res = getIntValue(node, all(decls), -1);
    Nest_freeNodeArray(decls);
    if (res > 0)
        return res;

    // Search the default precedence name
    decls = Nest_symTabLookup(node.context()->currentSymTab, defaultPrecedenceName);
    res = getIntValue(node, all(decls), -1);
    Nest_freeNodeArray(decls);
    if (res > 0)
        return res;

    return 0;
}

bool isRightAssociativity(InfixOp node) {
    string assocName = "oper_assoc_" + node.oper().toStd();

    // Perform a name lookup for the actual associativity name
    auto decls = Nest_symTabLookup(node.context()->currentSymTab, assocName.c_str());
    int res = getIntValue(node, all(decls), 1);
    Nest_freeNodeArray(decls);
    return res < 0;
}

void handlePrecedence(InfixOp node) {
    ASSERT(node);

    // We go from left to right, trying to fix the precedence
    // Usually the tree is is deep on the left side, but it can contain some sub-trees on the right
    // side too
    for (; /*ever*/;) {
        int rankCur = getPrecedence(node);

        // Check right wing first
        InfixOp rightOp = node.arg2().kindCast<InfixOp>();
        if (rightOp) {
            int rankRight = getPrecedence(rightOp);
            // We never swap right if on the right we have a prefix operator (no 1st arg)
            if (rankRight <= rankCur && rightOp.arg1()) {
                swapRight(node);
                continue;
            }
        }

        InfixOp leftOp = node.arg1().kindCast<InfixOp>();
        if (leftOp) {
            handlePrecedence(leftOp);

            int rankLeft = getPrecedence(leftOp);
            if (rankLeft < rankCur) {
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

void handleAssociativity(InfixOp node) {
    // Normally, we should only check the left children for right associativity.
    // But, as we apply precedence, we must check for associativity in both directions

    ASSERT(node);

    StringRef curOp = node.oper();
    bool isRightAssoc = isRightAssociativity(node);
    if (!isRightAssoc) {
        for (; /*ever*/;) {
            InfixOp rightOp = node.arg2().kindCast<InfixOp>();
            if (!rightOp)
                break;

            handleAssociativity(rightOp);
            // We never swap right if on the right we have a prefix operator (no 1st arg)
            if (rightOp && curOp == rightOp.oper() && rightOp.arg1())
                swapRight(node);
            else
                break;
        }
    } else {
        for (; /*ever*/;) {
            InfixOp leftOp = node.arg1().kindCast<InfixOp>();
            if (!leftOp)
                break;

            handleAssociativity(leftOp);
            if (leftOp && curOp == leftOp.oper())
                swapLeft(node);
            else
                break;
        }
    }
}
} // namespace

DEFINE_NODE_COMMON_IMPL(Literal, NodeHandle)

Literal Literal::create(const Location& loc, StringRef litType, StringRef data) {
    auto res = Nest::createNode<Literal>(loc);
    res.setProperty(propSprLiteralType, litType);
    res.setProperty(propSprLiteralData, data);
    return res;
}

bool Literal::isString() const { return typeStr() == "StringRef"; }

NodeHandle Literal::semanticCheckImpl(Literal node) {
    StringRef litType = node.typeStr();
    StringRef data = node.dataStr();

    // Get the type of the literal by looking up the type name
    auto ident = Identifier::create(node.location(), litType);
    ident.setContext(node.context());
    if (!ident.computeType())
        return {};
    auto t = TypeWithStorage(getType(ident));
    t = t.changeMode(modeCt, node.location());

    if (litType == "StringRef") {
        // Create the explanation
        return Feather_mkCtValueT(node.location(), t, &data);
    } else
        return Feather::CtValueExp::create(node.location(), t, data);
}

DEFINE_NODE_COMMON_IMPL(Identifier, NodeHandle)

Identifier Identifier::create(const Location& loc, StringRef id) {
    auto res = Nest::createNode<Identifier>(loc);
    res.setProperty("name", id);
    return res;
}

NodeHandle Identifier::semanticCheckImpl(Identifier node) {
    StringRef id = node.name();
    const Location& loc = node.location();

    // Search in the current symbol table for the identifier
    auto declsOrig = Nest_symTabLookup(node.context()->currentSymTab, id.begin);
    // If not found, check if we may have an implicit this
    // Try searching for the symbol in the context of the 'this' datatype
    if (size(declsOrig) == 0) {
        Feather::FunctionDecl funDecl = Feather_getParentFun(node.context());
        if (funDecl) {
            // Does the function has a 'this' param?
            auto thisParamType = funDecl.getPropertyDefaultType(propThisParamType, Type{});
            if (thisParamType && thisParamType.hasStorage()) {
                NodeHandle dataType = thisParamType.referredNode();
                if (dataType) {
                    declsOrig = Nest_symTabLookupCurrent(
                            dataType.childrenContext()->currentSymTab, id.begin);
                }
            }
        }
    }

    if (size(declsOrig) == 0)
        REP_ERROR_RET(nullptr, loc, "No declarations found with the given name (%1%)") % id;

    // At this point, 'declsOrig' may contain using nodes that will point to other decls
    // Walk over our decls and try to get to the final referred decls
    // Also filter our nodes that cannot be access or without a proper type
    auto decls = expandDecls(all(declsOrig), node);

    if (size(decls) == 0) {
        REP_ERROR(loc, "No compatible declarations found with the given name (%1%)") % id;

        // Print the removed declarations
        for (NodeHandle n : declsOrig) {
            if (n && n.type())
                REP_INFO(n.location(), "See inaccessible declaration");
            else if (n && n.hasError())
                REP_INFO(n.location(), "See declaration with error");
            else if (n && !n.type())
                REP_INFO(n.location(), "See declaration without a type");
            else
                REP_INFO(loc, "Encountered invalid declaration");
        }
        Nest_freeNodeArray(declsOrig);
        Nest_freeNodeArray(decls);
        return {};
    }

    // If at least one decl is a field or method, then transform this into a compound expression
    // starting from 'this'
    bool needsThis = false;
    for (DeclNode decl : decls) {
        ASSERT(decl.type());
        auto expl = decl.explanation();
        if (isField(expl)) {
            needsThis = true;
            break;
        }
    }
    if (needsThis) {
        // Add 'this' parameter to handle this case
        return CompoundExp::create(loc, Identifier::create(loc, "this"), id);
    }

    bool allowDeclExp = 0 != node.getPropertyDefaultInt(propAllowDeclExp, 0);
    auto res = getIdentifierResult(node, all(decls), nullptr, allowDeclExp);
    Nest_freeNodeArray(decls);
    Nest_freeNodeArray(declsOrig);
    return res;
}

DEFINE_NODE_COMMON_IMPL(CompoundExp, NodeHandle)

CompoundExp CompoundExp::create(const Location& loc, NodeHandle base, StringRef id) {
    auto res = Nest::createNode<CompoundExp>(loc, NodeRange({base}));
    res.setProperty("name", id);
    return res;
}

NodeHandle CompoundExp::semanticCheckImpl(CompoundExp node) {
    NodeHandle base = node.base();
    StringRef id = node.name();

    // For the base expression allow it to return DeclExp
    base.setPropertyExpl(propAllowDeclExp, 1);

    // Compile the base expression
    // We can expect at the base node both traditional expressions and nodes yielding decl-type
    // types
    if (!base.semanticCheck())
        return {};

    // Try to get the declarations pointed by the base node
    Node* baseDataExp1 = nullptr;
    NodeVector baseDecls = getDeclsFromNode(base, baseDataExp1);
    NodeHandle baseDataExp = baseDataExp1;

    // If the base has storage, retain it as the base data expression
    if (baseDecls.empty() && base.type().hasStorage())
        baseDataExp = base;
    if (baseDataExp && !baseDataExp.computeType())
        return {};
    node.setProperty("baseDataExp", baseDataExp);

    // Get the declarations that this node refers to
    Nest_NodeArray declsOrig{nullptr, nullptr, nullptr};
    if (!baseDecls.empty()) {
        // Get the referred declarations; search for our id inside the symbol table of the
        // declarations of the base

        // Get the decls from the first base decl
        if (baseDecls[0]->childrenContext)
            declsOrig = Nest_symTabLookupCurrent(
                    baseDecls[0]->childrenContext->currentSymTab, id.begin);

        // And now from the rest of base decls
        for (size_t i = 1; i < baseDecls.size(); i++) {
            Node* baseDecl = baseDecls[i];
            if (!baseDecl->childrenContext)
                continue;
            auto declsCur =
                    Nest_symTabLookupCurrent(baseDecl->childrenContext->currentSymTab, id.begin);
            Nest_appendNodesToArray(&declsOrig, all(declsCur));
            Nest_freeNodeArray(declsCur);
        }
    } else if (base.type().hasStorage()) {
        // Search for the decls from datatype
        string idWithPrefixStr = "post_" + StringRef(id).toStd();
        StringRef idPrefix = StringRef(idWithPrefixStr);
        declsOrig = searchDeclsFromDatatype(node.context(), base, id, idPrefix);

        // If nothing was found, check if there is a dot operator to use
        if (size(declsOrig) == 0) {
            // Search for the dot operator near the datatype
            declsOrig = searchDeclsFromDatatype(
                    node.context(), base, ".", StringRef{}, SearchType::nearOnly);

            // Transform 'base' into 'base .'
            Location loc = base.location();
            auto faBase = DeclExp::create(loc, all(declsOrig), baseDataExp);
            baseDataExp = FunApplication::create(loc, faBase, nullptr);
            baseDataExp.setContext(base.context());

            // Not needing the decls anymore
            Nest_freeNodeArray(declsOrig);

            // Now, we need to perform another search from the current base
            if (baseDataExp && !baseDataExp.computeType())
                return {};

            // Perform another search from this point, to find the intended decl
            declsOrig = searchDeclsFromDatatype(node.context(), baseDataExp, id, idPrefix);
        }
    }

    // At this point, 'declsOrig' may contain using nodes that will point to other decls
    // Walk over our decls and try to get to the final referred decls
    // Also filter our nodes that cannot be access or without a proper type
    auto decls = expandDecls(all(declsOrig), node);

    // Error reporting
    if (size(decls) == 0) {
        // Get the string describing where we are searching
        ostringstream oss;
        if (baseDecls.empty()) {
            oss << base << ": " << base.type();
        } else if (baseDecls.size() == 1) {
            oss << DeclNode(baseDecls[0]).name().toStd();
        } else {
            oss << "decls(";
            for (unsigned i = 0; i < baseDecls.size(); ++i) {
                if (i > 0)
                    oss << ", ";
                oss << DeclNode(baseDecls[i]).name().toStd();
            }
            oss << ")";
        }
        REP_ERROR(node.location(), "No declarations found with the name '%1%' inside %2%") % id %
                oss.str();

        if (size(declsOrig) == 0)
            for (NodeHandle n : baseDecls)
                REP_INFO(n.location(), "See searched declaration");

        // Print the removed declarations
        for (NodeHandle n : declsOrig) {
            if (n && n.type())
                REP_INFO(n.location(), "See inaccessible declaration");
            else if (n && n.hasError())
                REP_INFO(n.location(), "See declaration with error");
            else if (n && !n.type())
                REP_INFO(n.location(), "See declaration without a type");
            else
                REP_INFO(node.location(), "Encountered invalid declaration");
        }
        Nest_freeNodeArray(declsOrig);
        Nest_freeNodeArray(decls);
        return {};
    }

    bool allowDeclExp = 0 != node.getPropertyDefaultInt(propAllowDeclExp, 0);
    return getIdentifierResult(node, all(decls), baseDataExp, allowDeclExp);
}
const char* CompoundExp::toStringImpl(CompoundExp node) {
    auto base = node.base();
    StringRef id = node.name();

    ostringstream os;
    os << base << "." << id;
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(FunApplication, NodeHandle)

FunApplication FunApplication::create(const Location& loc, NodeHandle base, NodeList arguments) {
    if (!arguments)
        arguments = NodeList::create(loc, NodeRange{}, true);
    return Nest::createNode<FunApplication>(loc, NodeRange({base, arguments}));
}

FunApplication FunApplication::create(const Location& loc, NodeHandle base, NodeRange arguments) {
    auto argsNode = NodeList::create(loc, arguments, true);
    return Nest::createNode<FunApplication>(loc, NodeRange({base, argsNode}));
}

FunApplication FunApplication::create(const Location& loc, StringRef fname, NodeRange arguments) {
    auto base = Identifier::create(loc, fname);
    auto argsNode = NodeList::create(loc, arguments, true);
    return Nest::createNode<FunApplication>(loc, NodeRange({base, argsNode}));
}

NodeHandle FunApplication::semanticCheckImpl(FunApplication node) {
    auto base = node.base();
    auto arguments = node.arguments();
    const Location& loc = node.location();

    if (!base)
        REP_INTERNAL(loc, "Don't know what function to call");
    ASSERT(base);

    // For the base expression allow it to return DeclExp
    base.setPropertyExpl(propAllowDeclExp, 1);

    // Compile the base expression
    // We can expect here both traditional expressions and nodes yielding decl-type types
    if (!base.semanticCheck())
        return {};

    // Check for Sparrow implicit functions
    Identifier ident = base.kindCast<Identifier>();
    if (ident) {
        StringRef id = ident.name();
        if (id == "isValid") {
            return checkIsValid(node);
        } else if (id == "isValidAndTrue") {
            return checkIsValidAndTrue(node);
        } else if (id == "valueIfValid") {
            return checkValueIfValid(node);
        } else if (id == "typeOf") {
            return checkTypeOf(node);
        } else if (id == "astLift") {
            return checkAstLift(node);
        } else if (id == "ife") {
            return checkIfe(node);
        }
    }

    // Compile the arguments
    if (arguments && !arguments.semanticCheck())
        return {};
    if (arguments && arguments.hasError())
        REP_INTERNAL(loc, "Args with error");

    // Check for Sparrow implicit functions
    if (ident) {
        StringRef id = ident.name();
        if (id == "cast") {
            return checkStaticCast(node);
        } else if (id == "reinterpretCast") {
            return checkReinterpretCast(node);
        } else if (id == "sizeOf") {
            return checkSizeOf(node);
        } else if (id == "ctEval") {
            return checkCtEval(node);
        } else if (id == "construct") {
            return checkConstruct(node, arguments.children());
        }
    }

    // Try to get the declarations pointed by the base node
    Node* thisArg1 = nullptr;
    NodeVector decls = getDeclsFromNode(base, thisArg1);
    NodeHandle thisArg = thisArg1;

    string functionName;
    if (!decls.empty())
        functionName = DeclNode(decls.front()).name().toStd();
    else
        functionName = base.toString();

    // If we didn't find any declarations, try the operator call
    if (base.type().hasStorage() && decls.empty()) {
        Feather::StructDecl structDecl = Feather::StructDecl(base.type().referredNode());
        auto arr = getClassAssociatedDecls(structDecl, "()");
        decls = Nest::toVec(arr);
        Nest_freeNodeArray(arr);
        if (decls.empty())
            REP_ERROR_RET(nullptr, loc, "Class %1% has no user defined call operators") %
                    structDecl.name();
        thisArg = base;
        functionName = "()";
    }

    // The name of function we are trying to call
    if (functionName == base.kindName())
        functionName = "function";

    // The arguments to be used, including thisArg
    NodeVector args;
    if (thisArg)
        args.push_back(thisArg);
    if (arguments)
        args.insert(args.end(), begin(arguments.children()), end(arguments.children()));

    // Check the right overload based on the type of the arguments
    EvalMode mode = node.context()->evalMode;
    if (thisArg)
        mode = Feather_combineMode(thisArg.type().mode(), mode);
    Node* res = g_OverloadService->selectOverload(node.context(), loc, mode, all(decls), all(args),
            OverloadReporting::full, StringRef(functionName));

    return res;
}

DEFINE_NODE_COMMON_IMPL(OperatorCall, NodeHandle)

OperatorCall OperatorCall::create(
        const Location& loc, NodeHandle arg1, StringRef op, NodeHandle arg2) {
    if (op.empty())
        REP_ERROR_RET(nullptr, loc, "Invalid operation name");
    auto res = Nest::createNode<OperatorCall>(loc, NodeRange({arg1, arg2}));
    res.setProperty(propSprOperation, op);
    return res;
}

NodeHandle OperatorCall::semanticCheckImpl(OperatorCall node) {
    auto arg1 = node.arg1();
    auto arg2 = node.arg2();
    StringRef operation = node.oper();

    if (arg1 && arg2) {
        if (operation == "===")
            return handleRefEq(node);
        else if (operation == "!==")
            return handleRefNe(node);
        else if (operation == ":=")
            return handleRefAssign(node);
    }
    if (arg2 && !arg1 && operation == "\\")
        return handleFunPtr(node); // TODO: this should ideally be defined in std lib
    if (operation == "construct")
        return checkConstruct(node, node.children());

    // Search for the operator
    NodeHandle res = selectOperator(node, operation, arg1, arg2, false);

    // If nothing found try fall-back.
    if (!res && arg1) {
        string op1, op2;
        auto a1 = arg1;
        auto a2 = arg2;

        if (arg2) {
            if (operation == "!=") // Transform '!=' into '=='
            {
                op1 = "==";
                op2 = "!";
            } else if (operation == ">") // Transform '>' into '<'
            {
                op1 = "<";
                a1 = arg2;
                a2 = arg1;
            } else if (operation == "<=") // Transform '<=' into '<'
            {
                op1 = "<";
                op2 = "!";
                a1 = arg2;
                a2 = arg1;
            } else if (operation == ">=") // Transform '>=' into '<'
            {
                op1 = "<";
                op2 = "!";
            }

            // Check for <op>= operators
            else if (operation.size() >= 2 && operation.begin[operation.size() - 1] == '=') {
                StringRef baseOperation = operation;
                baseOperation.end--;

                // Transform 'a <op>= b' into 'a = a <op> b'
                op1 = baseOperation.toStd();
                op2 = "=";
            }
        }

        // Fallback case: attempt to apply the dot operator
        if (op1.empty()) {
            op1 = ".";
            a1 = arg1;
            a2 = nullptr;
        }

        if (!op1.empty()) {
            NodeHandle r = selectOperator(node, StringRef(op1), a1, a2, false);
            if (r) {
                if (!r.semanticCheck())
                    return {};
                if (op2 == "!")
                    res = selectOperator(node, StringRef(op2), r, nullptr, false);
                else if (op2 == "=")
                    res = selectOperator(node, StringRef(op2), a1, r, false);
                else if (op1 == ".") {
                    res = selectOperator(node, operation, r, arg2, false);
                } else if (op2.empty())
                    res = r;
            }
        }
    }

    if (!res) {
        if (arg1 && arg2)
            REP_ERROR(node.location(), "Cannot find an overload for calling operator %2% %1% %3%") %
                    operation % argTypeStr(arg1) % argTypeStr(arg2);
        else if (arg1)
            REP_ERROR(node.location(), "Cannot find an overload for calling operator %2% %1%") %
                    operation % argTypeStr(arg1);
        else if (arg2)
            REP_ERROR(node.location(), "Cannot find an overload for calling operator %1% %2%") %
                    operation % argTypeStr(arg2);
        else
            REP_ERROR(node.location(), "Cannot find an overload for calling operator %1%") %
                    operation;

        // Try now once more to select the operator, now with errors turned on
        selectOperator(node, operation, arg1, arg2, true);

        return {};
    }

    return res;
}

DEFINE_NODE_COMMON_IMPL(InfixOp, NodeHandle)

InfixOp InfixOp::create(const Location& loc, StringRef op, NodeHandle arg1, NodeHandle arg2) {
    if (op.empty())
        REP_ERROR_RET(nullptr, loc, "Invalid infix operation name");
    auto res = Nest::createNode<InfixOp>(loc, NodeRange({arg1, arg2}));
    res.setProperty(propSprOperation, op);
    return res;
}

NodeHandle InfixOp::semanticCheckImpl(InfixOp node) {

    // This is constructed in such way that left most operations are applied first.
    // This way, we have a tree that has a lot of children on the left side and one children on the
    // right side When applying the precedence and associativity rules for the expressions in here,
    // we will try to balance this tree, and move some elements to the right side of the tree by
    // performing some swaps

    handlePrecedence(node);
    handleAssociativity(node);

    return OperatorCall::create(node.location(), node.arg1(), node.oper(), node.arg2());
}

DEFINE_NODE_COMMON_IMPL(LambdaExp, NodeHandle)

LambdaExp LambdaExp::create(const Location& loc, NodeList parameters, NodeHandle returnType,
        NodeHandle body, NodeHandle bodyExp, NodeList closureParams) {
    if (bodyExp && body)
        REP_INTERNAL(loc, "Cannot construct a LambdaExp with both a body and a body expression");
    if (bodyExp) {
        // Translate <bodyExp> into { return <bodyExp>; }
        const Location& locBody = bodyExp.location();
        body = Feather::LocalSpace::create(
                locBody, NodeRange({ReturnStmt::create(locBody, bodyExp)}));

        // If we don't have a type given, infer it from the given expression
        if (!returnType)
            returnType = FunApplication::create(locBody, "typeOf", NodeRange({bodyExp}));
    }
    return Nest::createNode<LambdaExp>(
            loc, NodeRange({}), NodeRange({parameters, returnType, body, closureParams}));
}

NodeHandle LambdaExp::semanticCheckImpl(LambdaExp node) {
    auto parameters = node.parameters();
    auto returnType = node.returnType();
    auto body = node.body();
    auto closureParams = node.closureParams();
    Location loc = node.location();

    // This will expand to:
    //
    // package $lambdaEnclosurePackage
    //      datatype $lambdaEnclosureData
    //          <closureParams> // as fields
    //      fun ()(this: @ $lambdaEnclosureData, <parameters>)
    //          <body>

    // Create the enclosing class body node list;
    // Add here fields corresponding to the closure params
    auto datatypeBody = NodeList::create(loc, NodeRange{}, true);
    if (closureParams) {
        for (auto arg : closureParams.children()) {
            // Get the name and the type of the argument
            if (!arg)
                REP_INTERNAL(closureParams.location(), "Invalid closure parameter");
            ASSERT(arg);
            auto id = arg.kindCast<Identifier>();
            if (!id)
                REP_INTERNAL(arg.location(), "The closure parameter must be an identifier");
            id.setContext(node.context());
            if (!id.semanticCheck())
                return {};
            StringRef varName = id.name();
            Type varType = Feather::removeCategoryIfPresent(arg.type());

            // Create a similar variable in the enclosing class - must have the same name
            const Location& argLoc = arg.location();
            auto typeNode = createTypeNode(nullptr, argLoc, varType);
            datatypeBody.addChild(FieldDecl::create(argLoc, varName, typeNode));
        }
    }

    // Create the lambda closure class
    auto closureDatatype = DataTypeDecl::create(
            loc, "$lambdaEnclosureData", nullptr, nullptr, nullptr, datatypeBody);
    if (closureParams && !closureParams.children().empty())
        closureDatatype.setProperty(propGenerateInitCtor, 1);

    // The actual enclosed function -- ensure adding a 'this' parameter
    auto thisParamTypeNode = Identifier::create(loc, "$lambdaEnclosureData");
    auto thisParam = ParameterDecl::create(loc, "this", thisParamTypeNode);
    auto parametersWithThis = NodeList::create(loc, NodeRange({thisParam}), true);
    NodeList::append(parametersWithThis, parameters);
    auto enclosedFun = SprFunctionDecl::create(loc, "()", parametersWithThis, returnType, body);

    // Create the enclosing package node list
    auto packageBody = NodeList::create(loc, NodeRange({closureDatatype, enclosedFun}), true);
    auto closurePackage = PackageDecl::create(loc, "$lambdaEnclosurePackage", packageBody);

    // Set the right eval mode for the closure package
    EvalMode evalMode = Feather_effectiveEvalMode(node);
    closurePackage.setMode(evalMode);

    // Add the closure package as a top level node of this node
    // We add it in the context of the parent function
    Feather::FunctionDecl parentFun = Feather_getParentFun(node.context());
    CompilationContext* parentContext = parentFun ? parentFun.context() : node.context();
    closurePackage.setContext(parentContext);
    node.addAdditionalNode(closurePackage);

    // Semantically check the resulting package
    if (!closurePackage.semanticCheck())
        return {};

    // Create a resulting object: a constructor call to our class
    Feather::StructDecl datatypeDecl = Feather::StructDecl(closureDatatype.explanation());
    auto datatype = Feather::DataType::get(datatypeDecl, modeRt);
    auto classId = createTypeNode(node.context(), loc, datatype);
    return FunApplication::create(loc, classId, closureParams);
}

DEFINE_NODE_COMMON_IMPL(ConditionalExp, NodeHandle)

ConditionalExp ConditionalExp::create(
        const Location& loc, NodeHandle cond, NodeHandle alt1, NodeHandle alt2) {
    REQUIRE_NODE(loc, cond);
    REQUIRE_NODE(loc, alt1);
    REQUIRE_NODE(loc, alt2);
    return Nest::createNode<ConditionalExp>(loc, NodeRange({cond, alt1, alt2}));
}

NodeHandle ConditionalExp::semanticCheckImpl(ConditionalExp node) {
    NodeHandle cond = node.cond();
    NodeHandle alt1 = node.alt1();
    NodeHandle alt2 = node.alt2();

    if (!alt1.semanticCheck() || !alt2.semanticCheck())
        return {};

    Type t1 = alt1.type();
    Type t2 = alt2.type();

    // Get the common type
    Type resType = commonType(node.context(), t1, t2);
    if (resType == StdDef::typeVoid)
        REP_ERROR_RET(nullptr, node.location(),
                "Cannot deduce the result type for a conditional expression (%1%, %2%)") %
                t1 % t2;

    // Convert both types to the result type
    ConversionResult c1 = g_ConvertService->checkConversion(node.context(), t1, resType);
    ConversionResult c2 = g_ConvertService->checkConversion(node.context(), t2, resType);

    alt1 = c1.apply(node.context(), alt1);
    alt2 = c2.apply(node.context(), alt2);

    return Feather::ConditionalExp::create(node.location(), cond, alt1, alt2);
}

DEFINE_NODE_COMMON_IMPL(DeclExp, NodeHandle)

DeclExp DeclExp::create(const Location& loc, NodeRange decls, NodeHandle baseExp) {
    auto res = Nest::createNode<DeclExp>(loc, NodeRange({}), NodeRange({baseExp}));
    res.addReferredNodes(decls);
    return res;
}

NodeRange DeclExp::referredDecls() const {
    // At position 0, we have the base expression
    return referredNodes().skip(1);
}

NodeHandle DeclExp::semanticCheckImpl(DeclExp node) {
    // Make sure we computed the type for all the referred nodes
    for (auto n : node.referredNodes()) {
        if (n && !n.computeType())
            continue;
    }
    node.setType(Feather::VoidType::get(node.context()->evalMode));
    return node; // This node should never be translated directly
}

DEFINE_NODE_COMMON_IMPL(StarExp, NodeHandle)

StarExp StarExp::create(const Location& loc, NodeHandle baseExp, StringRef operName) {
    if (operName != "*")
        REP_ERROR_RET(nullptr, loc, "Expected '*' in expression; found '%1%'") % operName;
    return Nest::createNode<StarExp>(loc, NodeRange({baseExp}));
}

NodeHandle StarExp::semanticCheckImpl(StarExp node) {
    NodeHandle base = node.baseExp();

    // For the base expression allow it to return DeclExp
    base.setPropertyExpl(propAllowDeclExp, 1);

    // Get the declarations from the base expression
    if (!base.semanticCheck())
        return {};
    Node* baseExp;
    NodeVector baseDecls = getDeclsFromNode(base, baseExp);
    if (baseDecls.empty())
        REP_ERROR_RET(nullptr, base.location(),
                "Invalid expression inside star expression (no referred declarations)");

    // Get the referred declarations
    NodeVector decls;
    for (NodeHandle baseDecl : baseDecls) {
        if (!baseDecl)
            continue;

        // Get the sym tab from the base declaration
        Nest_SymTab* baseSymTab = Nest_childrenContext(baseDecl)->currentSymTab;
        if (!baseSymTab)
            continue;

        // Get all the symbols from the symbol table

        // Search in the symbol table of the base for the identifier
        auto newEntries = Nest::toVec(Nest_symTabAllEntries(baseSymTab));
        decls.insert(decls.begin(), newEntries.begin(), newEntries.end());
    }

    // Make sure the declarations are unique
    sort(decls.begin(), decls.end());
    decls.erase(unique(decls.begin(), decls.end()), decls.end());

    // Remove all the inaccessible entries
    for (Node*& decl : decls) {
        if (!canAccessNode(decl, node))
            decl = nullptr;
    }
    auto it = remove_if(decls.begin(), decls.end(), [](Node* d) { return d == nullptr; });
    decls.erase(it, decls.end());

    // This expands to a declaration expression
    return DeclExp::create(node.location(), all(decls));
}
} // namespace SprFrontend
