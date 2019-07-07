#include "SparrowFrontend/StdInc.h"
#include "SparrowFrontend/Nodes/Generics.hpp"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"

#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Utils/cppif/NodeHelpers.hpp"

namespace SprFrontend {

DEFINE_NODE_COMMON_IMPL(GenericPackage, DeclNode)

GenericPackage GenericPackage::create(
        PackageDecl original, NodeList parameters, NodeHandle ifClause) {

    auto instSet = InstantiationsSet::create(original, parameters.children(), ifClause);

    auto res = Nest::createNode<GenericPackage>(
            original.location(), NodeRange({instSet}), NodeRange({original}));

    res.setNameAndMode(original.name(), original.effectiveMode());
    copyAccessType(res, original);
    res.addAdditionalNode(original);

    // Semantic check the arguments
    for (auto param : parameters.children()) {
        if (!param.semanticCheck())
            return {};
        if (isConceptType(param.type()))
            REP_ERROR_RET(nullptr, param.location(),
                    "Cannot use auto or concept parameters for package generics");
        if (param.type().mode() != modeCt)
            REP_ERROR_RET(nullptr, param.location(), "Generic package parameters must be CT");
    }

    return res;
}

InstantiationsSet GenericPackage::instSet() const { return {children()[0]}; }

NodeHandle GenericPackage::semanticCheckImpl(GenericPackage node) {
    return Feather::Nop::create(node.location());
}

DEFINE_NODE_COMMON_IMPL(GenericDatatype, DeclNode)

GenericDatatype GenericDatatype::create(
        DataTypeDecl original, NodeList parameters, NodeHandle ifClause) {
    auto instSet = InstantiationsSet::create(original, parameters.children(), ifClause);

    auto res = Nest::createNode<GenericDatatype>(
            original.location(), NodeRange({instSet}), NodeRange({original}));
    copyAccessType(res, original);
    res.setNameAndMode(original.name(), original.effectiveMode());
    res.addAdditionalNode(original);

    // Semantic check the arguments
    for (auto param : parameters.children()) {
        if (!param.semanticCheck())
            return {};
        if (isConceptType(param.type()))
            REP_ERROR_RET(nullptr, param.location(),
                    "Cannot use auto or concept parameters for class generics");
        if (param.type().mode() != modeCt)
            REP_ERROR_RET(nullptr, param.location(), "Generic datatype parameters must be CT");
    }

    return res;
}

InstantiationsSet GenericDatatype::instSet() const { return {children()[0]}; }

NodeHandle GenericDatatype::semanticCheckImpl(GenericDatatype node) {
    return Feather::Nop::create(node.location());
}

DEFINE_NODE_COMMON_IMPL(GenericFunction, DeclNode)

GenericFunction GenericFunction::create(
        SprFunctionDecl original, NodeRange params, NodeRange genericParams, NodeHandle ifClause) {
    const Location& loc = original.location();
    auto instSet = InstantiationsSet::create(original, genericParams, ifClause);
    auto paramsNode = NodeList::create(loc, params, true);

    auto res = Nest::createNode<GenericFunction>(
            loc, NodeRange({instSet}), NodeRange({original, paramsNode}));

    copyAccessType(res, original);
    res.setNameAndMode(original.name(), original.effectiveMode());

    return res;
}

InstantiationsSet GenericFunction::instSet() const { return {children()[0]}; }

NodeHandle GenericFunction::semanticCheckImpl(GenericFunction node) {
    return Feather::Nop::create(node.location());
}

DEFINE_NODE_COMMON_IMPL(Instantiation, NodeHandle)

Instantiation Instantiation::create(
        const Location& loc, NodeRange boundValues, NodeRange boundVars) {

    auto paramsNode = Feather::NodeList::create(loc, boundVars, true);
    auto res = Nest::createNode<Instantiation>(loc, NodeRange({paramsNode}), boundValues);

    res.setProperty("instIsValid", 0);
    res.setProperty("instIsEvaluated", 0);
    res.setProperty("instantiatedDecl", NodeHandle{});

    return res;
}

NodeHandle Instantiation::semanticCheckImpl(Instantiation node) {
    return Feather::Nop::create(node.location());
}

DEFINE_NODE_COMMON_IMPL(InstantiationsSet, NodeHandle)

InstantiationsSet InstantiationsSet::create(
        Feather::DeclNode parentNode, NodeRange params, NodeHandle ifClause) {
    const Location& loc = parentNode.location();
    auto instsNl = Feather::NodeList::create(loc, NodeRange{}, true);
    auto paramsNode = Feather::NodeList::create(loc, params, true);
    auto res = Nest::createNode<InstantiationsSet>(
            loc, NodeRange({ifClause, instsNl}), NodeRange({parentNode, paramsNode}));
    return res;
}

void InstantiationsSet::addInstantiation(Instantiation inst) { childrenM()[1].addChild(inst); }

NodeHandle InstantiationsSet::semanticCheckImpl(InstantiationsSet node) {
    return Feather::Nop::create(node.location());
}

} // namespace SprFrontend
