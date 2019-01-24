#include "StdInc.h"
#include "SparrowFrontend/SprCommon/GenCallableDecl.hpp"
#include "SparrowFrontend/SprCommon/Utils.hpp"

#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Builder.h"
#include "SparrowFrontend/Nodes/SprProperties.h"
#include "Nest/Utils/cppif/StringRef.hpp"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

GenCallableDecl::GenCallableDecl(
        const Location& loc, CompilationContext* ctx, const SampleTypes& types)
    : paramsGenerator_(loc, ctx, {}, &types)
    , location_(loc)
    , context_(ctx)
    , types_(types) {}

SprFunctionDecl GenCallableDecl::genFunction(bool ifClauseVal) {
    paramsGenerator_ = GenGenericParams(location_, context_, {}, &types_);
    auto params = paramsGenerator_.genParameters();

    // Optionally, generate a return type
    NodeHandle returnType;
    if (randomChance(50)) {
        returnType = TypeNode::create(location_, *TypeFactory::arbDataType(modeRt));
    }

    // Leave the body empty
    NodeHandle body;

    // Add if clause only if it needs to be false
    NodeHandle ifClause;
    if (!ifClauseVal)
        ifClause = SprFrontend::buildBoolLiteral(location_, false);

    if (ifClause && !paramsGenerator_.isGeneric())
        return {};

    auto res = SprFunctionDecl::create(
            location_, "mySprFunctionDecl", params, returnType, body, ifClause);
    res.setContext(context_);
    return res;
}
PackageDecl GenCallableDecl::genGenPackage() {
    GenGenericParams::Options paramOptions;
    paramOptions.useRt = false;
    paramOptions.useConcept = false;
    paramOptions.useDependent = false;
    paramsGenerator_ = GenGenericParams(location_, context_, paramOptions, &types_);
    auto params = paramsGenerator_.genParameters();

    auto body = NodeList::create(location_, NodeRange{}, true);

    auto res = PackageDecl::create(location_, "MyPackageDecl", body, params);
    res.setContext(context_);
    return res;
}
DataTypeDecl GenCallableDecl::genGenDatatype() {
    GenGenericParams::Options paramOptions;
    paramOptions.useRt = false;
    paramOptions.useConcept = false;
    paramOptions.useDependent = false;
    paramsGenerator_ = GenGenericParams(location_, context_, paramOptions, &types_);
    auto params = paramsGenerator_.genParameters();
    auto body = NodeList::create(location_, NodeRange{}, true);
    auto res = DataTypeDecl::create(location_, "MyDatatypeDecl", params, {}, {}, body);
    res.setProperty(propNoDefault, 1);
    res.setContext(context_);
    return res;
}
DataTypeDecl GenCallableDecl::genConcreteDatatype() {
    // TODO
    return {};
}
ConceptDecl GenCallableDecl::genConcept() {
    auto numConcepts = int(TypeFactory::g_conceptDecls.size());
    auto idx = *rc::gen::inRange(0, numConcepts);
    return ConceptDecl(TypeFactory::g_conceptDecls[idx]);
}

NodeHandle GenCallableDecl::genCallableDecl() {
    int declKind = *rc::gen::weightedElement<int>({
            {10, 0},
            {1, 1},
            {2, 2},
            {7, 3},
            {3, 4},
    });
    switch (declKind) {
    case 0:
        return genFunction();
    case 1:
        return genFunction(false);
    case 2:
        return genGenPackage();
    case 3:
        return genGenDatatype();
    case 4:
        return genConcept();
    }
    return {};
}