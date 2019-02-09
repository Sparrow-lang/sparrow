#include "SparrowFrontend/StdInc.h"
#include "SparrowFrontend/Nodes/Stmt.hpp"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/CommonCode.h"
#include "SparrowFrontend/Services/IConvertService.h"

#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Utils/cppif/NodeHelpers.hpp"

namespace SprFrontend {

DEFINE_NODE_COMMON_IMPL(ForStmt, NodeHandle)

ForStmt ForStmt::create(const Location& loc, StringRef name, NodeHandle typeNode, NodeHandle range,
        NodeHandle action) {
    REQUIRE_NODE(loc, range);
    auto res = Nest::createNode<ForStmt>(loc, NodeRange({range, action, typeNode}));
    res.setProperty("name", name);
    return res;
}

void ForStmt::setContextForChildrenImpl(ForStmt node) {
    NodeHandle range = node.range();
    NodeHandle action = node.action();
    NodeHandle typeNode = node.typeNode();

    // a CT-for may induce a different context for typeNode and range
    auto ctx = node.context();
    auto rangeCtx = Feather_nodeEvalMode(node) == modeCt ? Nest_mkChildContext(ctx, modeCt) : ctx;
    if (typeNode)
        typeNode.setContext(rangeCtx);
    range.setContext(rangeCtx);

    // Keep the action in the same context
    if (action)
        action.setContext(ctx);
}

Type ForStmt::computeTypeImpl(ForStmt node) {
    return Feather::VoidType::get(node.context()->evalMode);
}

NodeHandle ForStmt::semanticCheckImpl(ForStmt node) {
    NodeHandle range = node.range();
    NodeHandle action = node.action();
    NodeHandle typeNode = node.typeNode();

    bool ctFor = Feather_nodeEvalMode(node) == modeCt;

    if (typeNode && !typeNode.semanticCheck())
        return {};
    if (!range.semanticCheck())
        return {};

    const Location& loc = range.location();

    if (ctFor && range.type().mode() != modeCt)
        REP_ERROR_RET(
                nullptr, loc, "Range must be available at CT, for a CT for (range type: %1%)") %
                range.type();

    // Expand the for statement of the form
    //      for ( <name>: <type> = <range> ) action;
    //
    // into:
    //      var $rangeVar: Range = <range>;
    //      while ( ! ($rangeVar isEmpty) ; $rangeVar popFront )
    //      {
    //          var <name>: <type> = $rangeVar front;
    //          action;
    //      }
    //
    // if <type> is not present, we will use '$rangeType.RetType'

    // Variable to hold the range - initialize it with the range node
    auto rangeVar = VariableDecl::create(
            loc, "$rangeVar", Identifier::create(loc, StringRef("Range")), range);
    if (ctFor)
        rangeVar.setMode(modeCt);
    auto rangeVarRef = Identifier::create(loc, StringRef("$rangeVar"));

    // while condition
    auto base1 = OperatorCall::create(loc, rangeVarRef, StringRef("isEmpty"), nullptr);
    auto whileCond = OperatorCall::create(loc, nullptr, StringRef("!"), base1);

    // while step
    auto whileStep = OperatorCall::create(loc, rangeVarRef, StringRef("popFront"), nullptr);

    // while body
    NodeHandle whileBody;
    if (action) {
        if (!typeNode)
            typeNode = CompoundExp::create(loc, rangeVarRef, StringRef("RetType"));

        // the iteration variable
        auto init = OperatorCall::create(loc, rangeVarRef, StringRef("front"), nullptr);

        auto iterVar = VariableDecl::create(node.location(), node.name(), typeNode, init);
        if (ctFor)
            iterVar.setMode(modeCt);

        whileBody = Feather::LocalSpace::create(action.location(), NodeRange({iterVar, action}));
    }

    auto whileStmt = Feather::WhileStmt::create(loc, whileCond, whileBody, whileStep);
    if (ctFor)
        whileStmt.setProperty(propEvalMode, (int)modeCt);

    return Feather::LocalSpace::create(node.location(), NodeRange({rangeVar, whileStmt}));
}

DEFINE_NODE_COMMON_IMPL(ReturnStmt, NodeHandle)

ReturnStmt ReturnStmt::create(const Location& loc, NodeHandle exp) {
    return Nest::createNode<ReturnStmt>(loc, NodeRange({exp}));
}

NodeHandle ReturnStmt::semanticCheckImpl(ReturnStmt node) {
    auto exp = node.exp();

    // Get the parent function of this return
    Feather::FunctionDecl parentFun = Feather_getParentFun(node.context());
    if (!parentFun)
        REP_ERROR_RET(nullptr, node.location(), "Return found outside any function");

    // Compute the result type of the function
    Type resType = nullptr;
    Feather::VarDecl resultParam = getResultParam(parentFun);
    if (resultParam) // Does this function have a result param?
    {
        resType = Feather::removeCatOrRef(TypeWithStorage(resultParam.type()));
        // As the result is passed as param-ref, the function should have void result
        ASSERT(!parentFun.resTypeNode().type().hasStorage());
    } else {
        resType = parentFun.resTypeNode().type();
    }
    ASSERT(resType);

    // Check match between return expression and function result type
    ConversionResult cvt = convNone;
    if (exp) {
        if (!exp.semanticCheck())
            return {};
        if (!resType.hasStorage() && exp.type() == resType) {
            return Feather::NodeList::create(node.location(),
                    NodeRange({exp, Feather::ReturnStmt::create(node.location(), nullptr)}));
        } else {
            cvt = g_ConvertService->checkConversion(exp, resType);
        }
        if (!cvt)
            REP_ERROR_RET(
                    nullptr, exp.location(), "Cannot convert return expression (%1%) to %2%") %
                    exp.type() % resType;
    } else {
        if (resultParam || parentFun.resTypeNode().type().kind() != typeKindVoid)
            REP_ERROR_RET(nullptr, node.location(),
                    "You must return something in a function that has non-Void result type");
    }

    // Build the explanation of this node
    if (resultParam) {
        // Create a ctor to construct the result parameter with the expression received
        const Location& l = resultParam.location();
        auto thisArg = Feather::MemLoadExp::create(l, Feather::VarRefExp::create(l, resultParam));
        thisArg.setContext(node.context());
        NodeHandle action = createCtorCall(l, node.context(), thisArg, exp);
        if (!action)
            REP_ERROR_RET(
                    nullptr, exp.location(), "Cannot construct return type object %1% from %2%") %
                    exp.type() % resType;

        return Feather::NodeList::create(node.location(),
                NodeRange({action, Feather::ReturnStmt::create(node.location(), nullptr)}));
    } else {
        exp = exp ? cvt.apply(node.context(), exp) : nullptr;
        return Feather::ReturnStmt::create(node.location(), exp);
    }
}
} // namespace SprFrontend
