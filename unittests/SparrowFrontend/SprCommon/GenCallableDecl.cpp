#include "StdInc.h"
#include "SparrowFrontend/SprCommon/GenCallableDecl.hpp"
#include "SparrowFrontend/SprCommon/Utils.hpp"
#include "Common/LocationGen.hpp"

#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Builder.h"
#include "SparrowFrontend/Nodes/SprProperties.h"
#include "Nest/Utils/cppif/StringRef.hpp"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

namespace {
SprFunctionDecl genFunction(const ParamsData& params, bool ifClauseVal) {
    // Optionally, generate a return type
    NodeHandle returnType;
    if (randomChance(50)) {
        returnType = TypeNode::create(g_LocationGen(), *TypeFactory::arbDataType(modeRt));
    }

    // Leave the body empty
    NodeHandle body;

    // Add if clause only if it needs to be false
    NodeHandle ifClause;
    if (!ifClauseVal)
        ifClause = SprFrontend::buildBoolLiteral(g_LocationGen(), false);

    if (ifClause && !params.isGeneric())
        return {};

    auto res = SprFunctionDecl::create(
            g_LocationGen(), "mySprFunctionDecl", params.paramsNode_, returnType, body, ifClause);
    return res;
}
} // namespace

rc::Gen<SprFunctionDecl> arbFunction(bool ifClauseVal) {
    return rc::gen::exec([=]() -> SprFunctionDecl {
        auto params = *arbParamsData();
        return genFunction(params, ifClauseVal);
    });
}

rc::Gen<SprFunctionDecl> arbFunction(const ParamsData& paramsData, bool ifClauseVal) {
    return rc::gen::exec([=]() -> SprFunctionDecl { return genFunction(paramsData, ifClauseVal); });
}

rc::Gen<PackageDecl> arbGenPackage() {
    return rc::gen::exec([]() -> PackageDecl {
        ParamsGenOptions paramOptions;
        paramOptions.useRt = false;
        paramOptions.useConcept = false;
        paramOptions.useDependent = false;
        auto params = *arbParamsData(paramOptions);

        auto body = NodeList::create(g_LocationGen(), NodeRange{}, true);

        auto res = PackageDecl::create(g_LocationGen(), "MyPackageDecl", body, params.paramsNode_);
        return res;
    });
}
rc::Gen<DataTypeDecl> arbGenDatatype() {
    return rc::gen::exec([]() -> DataTypeDecl {
        ParamsGenOptions paramOptions;
        paramOptions.useRt = false;
        paramOptions.useConcept = false;
        paramOptions.useDependent = false;
        auto params = *arbParamsData(paramOptions);
        auto body = NodeList::create(g_LocationGen(), NodeRange{}, true);
        auto res = DataTypeDecl::create(
                g_LocationGen(), "MyDatatypeDecl", params.paramsNode_, {}, {}, body);
        res.setProperty(propNoDefault, 1);
        return res;
    });
}
rc::Gen<DataTypeDecl> arbConcreteDatatype() {
    return rc::gen::exec([]() -> DataTypeDecl {
        // TODO
        return {};
    });
}
rc::Gen<ConceptDecl> arbConcept() {
    return rc::gen::exec([]() -> ConceptDecl {
        auto numConcepts = int(TypeFactory::g_conceptDecls.size());
        auto idx = *rc::gen::inRange(0, numConcepts);
        return ConceptDecl(TypeFactory::g_conceptDecls[idx]);
    });
}

rc::Gen<NodeHandle> arbCallableDecl() {
    return rc::gen::weightedOneOf<NodeHandle>({
            {10, rc::gen::cast<NodeHandle>(arbFunction())},
            {1, rc::gen::cast<NodeHandle>(arbFunction(false))},
            {2, rc::gen::cast<NodeHandle>(arbGenPackage())},
            {7, rc::gen::cast<NodeHandle>(arbGenDatatype())},
            {3, rc::gen::cast<NodeHandle>(arbConcept())},
    });
}
