#include "StdInc.h"
#include "Common/RcBasic.hpp"
#include "Common/TypeFactory.hpp"
#include "Common/GeneralFixture.hpp"
#include "Common/FeatherNodeFactory.hpp"
#include "SprCommon/SampleTypes.hpp"
#include "SprCommon/GenGenericParams.hpp"
#include "SprCommon/GenCallableDecl.hpp"
#include "SprCommon/Utils.hpp"

#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/Nodes/Builder.h"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

struct GenericsFixture : SparrowGeneralFixture {
    GenericsFixture();
    ~GenericsFixture();

    //! Checks if the created instantiation is OK, matching the parameters and given bound values
    static void checkInst(Instantiation inst, const ParamsData& paramsData, NodeRange values);

    //! The types that we are using while performing our tests
    SampleTypes types_;
};

GenericsFixture::GenericsFixture() {}

GenericsFixture::~GenericsFixture() {}

//! Checks if the created instantiation is OK, matching the parameters and given bound values
void GenericsFixture::checkInst(
        Instantiation inst, const ParamsData& paramsData, NodeRange values) {
    // The bound values must be the same
    RC_ASSERT(inst);
    RC_ASSERT(inst.boundValues().size() == int(values.size()));
    for (int i = 0; i < inst.boundValues().size(); i++) {
        auto instValue = inst.boundValues()[i].kindCast<CtValueExp>();
        if (instValue) {
            // For CTValues, check the value to be the same
            auto expectedValue = values[i].kindCast<CtValueExp>();
            RC_ASSERT(instValue);
            RC_ASSERT(expectedValue);
            RC_ASSERT(instValue.valueType() == expectedValue.valueType());
            RC_ASSERT(instValue.valueData() == expectedValue.valueData());
        } else {
            RC_ASSERT(inst.boundValues()[i] == values[i]);
        }
    }

    // Check the created bound var nodes
    RC_ASSERT(inst.boundVarsNode().children().size() <= int(values.size()));
    int boundVarIdx = 0;
    for (int i = 0; i < int(values.size()); i++) {
        // If we don't have a value, we should not have a variable; skip
        if (!values[i]) {
            continue;
        }

        NodeHandle boundVar = inst.boundVarsNode().children()[boundVarIdx++];
        RC_ASSERT(boundVar);
        RC_ASSERT(boundVar.kind() == Feather::VarDecl::staticKind() ||
                  boundVar.kind() == UsingDecl::staticKind());

        // Dependent type -- expect using with the type corresponding to the bound value
        // (dereferencing types on both sides)
        if (!paramsData.types_[i]) {
            auto usingDecl = boundVar.kindCast<UsingDecl>();
            RC_ASSERT(usingDecl);
            RC_ASSERT(usingDecl.name() == paramsData.params_[i].name());
            auto usedNode = usingDecl.usedNode();
            Type usedNodeType = getType(usedNode);
            Type expectedType = getType(values[i]);
            RC_ASSERT(usedNodeType == expectedType);
        }
        // If the parameter was a concept, expect a var-decl
        else if (paramsData.types_[i].kind() == typeKindConcept) {
            auto varDecl = boundVar.kindCast<Feather::VarDecl>();
            RC_ASSERT(varDecl);
            RC_ASSERT(varDecl.name() == paramsData.params_[i].name());
            // the type is a model of the concept; it should be the one from the bound value
            Type expectedType = getType(values[i]);
            RC_ASSERT(sameTypeIgnoreMode(varDecl.type(), expectedType));
        } else {
            // Regular case -- expect using of the right type
            RC_ASSERT(paramsData.types_[i].mode() == modeCt);

            auto usingDecl = boundVar.kindCast<UsingDecl>();
            RC_ASSERT(usingDecl);
            RC_ASSERT(usingDecl.name() == paramsData.params_[i].name());
            auto usedNode = usingDecl.usedNode();
            RC_ASSERT(usedNode.type() == values[i].type());
        }
    }
    RC_ASSERT(boundVarIdx == inst.boundVarsNode().children().size());
}

TEST_CASE_METHOD(GenericsFixture, "Test Generics.checkCreateGenericFun") {

    types_.init(*this);

    rc::prop("calling checkCreateGenericFun", [=]() {
        ParamsData paramsData = *arbParamsData();
        SprFunctionDecl fun = *arbFunction(paramsData);
        fun.setContext(globalContext_);

        RC_ASSERT(fun.parameters());
        auto res = checkCreateGenericFun(fun, fun.parameters().children(), fun.ifClause());

        // Now check the properties

        // If we have a concept, a CT type or a dependent type as parameter, we expect a generic
        bool expectResult = paramsData.usesConcepts() || paramsData.hasCtParams() ||
                            paramsData.hasDepedentParams();
        bool hasResult = !!res;
        RC_ASSERT(expectResult == hasResult);

        if (res) {
            // The original needs to point to the function passed in
            RC_ASSERT(res.original() == fun);

            // Check the original params; they need to be the same as the ones we passed in
            auto origParams = res.originalParams();
            auto ourParams = fun.parameters().children();
            RC_ASSERT(origParams.size() == ourParams.size());
            for (int i = 0; i < ourParams.size(); i++) {
                RC_ASSERT(origParams[i] == ourParams[i]);
            }

            // Check that we have an instantiations set, which is empty now
            InstantiationsSet instSet = res.instSet();
            RC_ASSERT(instSet);
            RC_ASSERT(!instSet.ifClause()); // we did not provide one
            RC_ASSERT(instSet.instantiations().empty());
            RC_ASSERT(instSet.parentNode() == fun);

            // Check the parameters of the inst set
            // We need to have the same number of parameters, but some of them may be null
            RC_ASSERT(instSet.params().size() == ourParams.size());
            for (int i = 0; i < ourParams.size(); i++) {
                auto t = ourParams[i].type();
                bool isRegular = t && t.kind() == typeKindData && t.mode() == modeRt;
                bool genericParamIsNull = !instSet.params()[i];
                RC_ASSERT(isRegular == genericParamIsNull);
            }
        }
    });
}

TEST_CASE_METHOD(GenericsFixture, "Test Generics.createNewInstantiation") {

    types_.init(*this, SampleTypes::onlyNumeric | SampleTypes::addByteType);

    rc::prop("resulting instantiation matches properties", [=]() {

        // Generate a generic function as the basis for our generics
        ParamsData paramsData = *arbParamsData();
        SprFunctionDecl fun = *arbFunction(paramsData);
        fun.setContext(globalContext_);

        // Get the instantiation set from it
        auto res = checkCreateGenericFun(fun, fun.parameters().children(), fun.ifClause());
        RC_PRE(res);
        InstantiationsSet instSet = res.instSet();
        RC_ASSERT(instSet);
        res.setContext(globalContext_);

        // Now generate some bound values -- several times
        constexpr int numGenerations = 10;
        for (int i = 0; i < numGenerations; i++) {
            // Generate random bound values
            auto values = *arbBoundValues(paramsData, types_);
            semanticCheck(values, globalContext_);
            EvalMode mode = *rc::gen::element(modeRt, modeCt, modeUnspecified);

            // try to create a new instantiation
            auto inst = createNewInstantiation(instSet, values, mode);

            // Check the instantiation
            checkInst(inst, paramsData, values);

            // At this point, the instantiation must be invalid and not evaluated
            RC_ASSERT(!inst.isValid());
            RC_ASSERT(!inst.isEvaluated());
        }
    });
}

TEST_CASE_METHOD(GenericsFixture, "Test Generics.canInstantiate OK") {
    types_.init(*this, SampleTypes::addByteType);

    rc::prop("canInstantiate will always return true", [=]() {

        // Generate a generic function as the basis for our generics
        ParamsData paramsData = *arbParamsData();
        SprFunctionDecl fun = *arbFunction(paramsData);
        fun.setContext(globalContext_);

        // Get the instantiation set from it
        auto res = checkCreateGenericFun(fun, fun.parameters().children(), fun.ifClause());
        RC_PRE(res);
        res.setContext(globalContext_);
        InstantiationsSet instSet = res.instSet();
        RC_ASSERT(instSet);

        // Now generate some bound values -- several times
        constexpr int numGenerations = 10;
        for (int i = 0; i < numGenerations; i++) {
            // Generate random bound values
            auto values = *arbBoundValues(paramsData, types_);
            semanticCheck(values, globalContext_);

            // Generate a random mode
            EvalMode mode = *rc::gen::element(modeRt, modeCt, modeUnspecified);

            // try to create a new instantiation
            auto inst = canInstantiate(instSet, values, mode);

            // Check the instantiation
            checkInst(inst, paramsData, values);

            // Instantiation must be valid and evaluated
            RC_ASSERT(inst.isValid());
            RC_ASSERT(inst.isEvaluated());

            // Instantiating again with the same parameters
            auto inst2 = canInstantiate(instSet, values, mode);
            // ... should yield exactly the same instantiation
            RC_ASSERT(inst == inst2);
        }
    });

    rc::prop("canInstantiate will fail when no values are passed in", [=]() {

        // Generate a generic function as the basis for our generics
        ParamsData paramsData = *arbParamsData();
        SprFunctionDecl fun = *arbFunction(paramsData);
        fun.setContext(globalContext_);

        // Get the instantiation set from it
        auto res = checkCreateGenericFun(fun, fun.parameters().children(), fun.ifClause());
        RC_PRE(res);
        InstantiationsSet instSet = res.instSet();
        RC_ASSERT(instSet);

        vector<NodeHandle> values;

        // try to create a new instantiation
        EvalMode mode = *rc::gen::element(modeRt, modeCt, modeUnspecified);
        auto inst = canInstantiate(instSet, values, mode);
        RC_ASSERT(!inst);
    });

    rc::prop("canInstantiate will fail on if-clause", [=]() {

        // Generate a generic function as the basis for our generics
        ParamsData paramsData = *arbParamsData();
        SprFunctionDecl fun = *arbFunction(paramsData, false);
        RC_PRE(fun);
        fun.setContext(globalContext_);

        // Get the instantiation set from it
        auto res = checkCreateGenericFun(fun, fun.parameters().children(), fun.ifClause());
        RC_PRE(res);
        InstantiationsSet instSet = res.instSet();
        RC_ASSERT(instSet);

        auto values = *arbBoundValues(paramsData, types_);
        semanticCheck(values, globalContext_);

        // try to create a new instantiation
        EvalMode mode = *rc::gen::element(modeRt, modeCt, modeUnspecified);
        auto inst = canInstantiate(instSet, values, mode);
        RC_ASSERT(!inst);
    });

    // TODO: Dependent params on CT params does not work
}
