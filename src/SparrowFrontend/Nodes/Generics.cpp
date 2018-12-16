#include "SparrowFrontend/StdInc.h"
#include "SparrowFrontend/Nodes/Generics.hpp"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
// #include "SparrowFrontend/Helpers/Convert.h"
// #include "SparrowFrontend/Helpers/CommonCode.h"

#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Utils/cppif/NodeHelpers.hpp"

namespace SprFrontend {

DEFINE_NODE_COMMON_IMPL(GenericPackage, DeclNode, nkSparrowDeclGenericPackage)

GenericPackage GenericPackage::create(
        PackageDecl original, NodeList parameters, NodeHandle ifClause) {

    auto instSet = InstantiationsSet::create(original, parameters.children(), ifClause);

    auto res = Nest::createNode<GenericPackage>(nkSparrowDeclGenericPackage, original.location(),
            NodeRange({instSet}), NodeRange({original}));

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
    }

    return res;
}

InstantiationsSet GenericPackage::instSet() const { return InstantiationsSet(children()[0]); }

NodeHandle GenericPackage::semanticCheckImpl2(GenericPackage node) {
    return Feather::Nop::create(node.location());
}

DEFINE_NODE_COMMON_IMPL(GenericDatatype, DeclNode, nkSparrowDeclGenericDatatype)

GenericDatatype GenericDatatype::create(
        DataTypeDecl original, NodeList parameters, NodeHandle ifClause) {
    auto instSet = InstantiationsSet::create(original, parameters.children(), ifClause);

    auto res = Nest::createNode<GenericDatatype>(nkSparrowDeclGenericDatatype, original.location(),
            NodeRange({instSet}), NodeRange({original}));
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
    }

    return res;
}

InstantiationsSet GenericDatatype::instSet() const { return InstantiationsSet(children()[0]); }

NodeHandle GenericDatatype::semanticCheckImpl2(GenericDatatype node) {
    return Feather::Nop::create(node.location());
}

DEFINE_NODE_COMMON_IMPL(GenericFunction, DeclNode, nkSparrowDeclGenericFunction)

GenericFunction GenericFunction::create(
        SprFunctionDecl original, NodeRange params, NodeRange genericParams, NodeHandle ifClause) {
    const Location& loc = original.location();
    auto instSet = InstantiationsSet::create(original, genericParams, ifClause);
    auto paramsNode = NodeList::create(loc, params, true);

    auto res = Nest::createNode<GenericFunction>(nkSparrowDeclGenericFunction, loc,
            NodeRange({instSet}), NodeRange({original, paramsNode}));

    copyAccessType(res, original);
    res.setNameAndMode(original.name(), original.effectiveMode());

    return res;
}

InstantiationsSet GenericFunction::instSet() const { return InstantiationsSet(children()[0]); }

NodeHandle GenericFunction::semanticCheckImpl2(GenericFunction node) {
    return Feather::Nop::create(node.location());
}

DEFINE_NODE_COMMON_IMPL(Instantiation, NodeHandle, nkSparrowInnerInstantiation)

Instantiation Instantiation::create(
        const Location& loc, NodeRange boundValues, NodeRange boundVars) {

    auto paramsNode = Feather::NodeList::create(loc, boundVars, true);
    auto res = Nest::createNode<Instantiation>(
            nkSparrowInnerInstantiation, loc, NodeRange({paramsNode}), boundValues);

    res.setProperty("instIsValid", 0);
    res.setProperty("instIsEvaluated", 0);
    res.setProperty("instantiatedDecl", NodeHandle{});

    return res;
}

NodeHandle Instantiation::semanticCheckImpl2(Instantiation node) {
    return Feather::Nop::create(node.location());
}

DEFINE_NODE_COMMON_IMPL(InstantiationsSet, NodeHandle, nkSparrowInnerInstantiationsSet)

InstantiationsSet InstantiationsSet::create(
        Feather::DeclNode parentNode, NodeRange params, NodeHandle ifClause) {
    const Location& loc = parentNode.location();
    auto instsNl = Feather::NodeList::create(loc, NodeRange{}, true);
    auto paramsNode = Feather::NodeList::create(loc, params, true);
    auto res = Nest::createNode<InstantiationsSet>(nkSparrowInnerInstantiationsSet, loc,
            NodeRange({ifClause, instsNl}), NodeRange({parentNode, paramsNode}));
    return res;
}

void InstantiationsSet::addInstantiation(Instantiation inst) { childrenM()[1].addChild(inst); }

NodeHandle InstantiationsSet::semanticCheckImpl2(InstantiationsSet node) {
    return Feather::Nop::create(node.location());
}

} // namespace SprFrontend
