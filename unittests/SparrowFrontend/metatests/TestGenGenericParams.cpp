#include "StdInc.h"
#include "Common/RcBasic.hpp"
#include "Common/TypeFactory.hpp"
#include "Common/GeneralFixture.hpp"
#include "SparrowFrontend/SprCommon/SampleTypes.hpp"
#include "SparrowFrontend/SprCommon/GenGenericParams.hpp"

#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

struct GenGenericParamsFixture : SparrowGeneralFixture {
    GenGenericParamsFixture();
    ~GenGenericParamsFixture();

    //! The types that we are using while performing our tests
    SampleTypes types_;
};

GenGenericParamsFixture::GenGenericParamsFixture() {}

GenGenericParamsFixture::~GenGenericParamsFixture() {}

TEST_CASE_METHOD(GenGenericParamsFixture, "Test GenGenericParams.genParams default") {
    types_.init(*this);
    rc::prop("params generated with genParameters match description", [=]() {
        GenGenericParams tested{GenGenericParams::Options{}};
        auto paramsList = tested.genParameters(Location());

        // Now check the returned parameters

        auto params = NodeRangeT<ParameterDecl>(paramsList.children());
        const auto& innerData = tested.innerData();

        // The parameters match
        RC_ASSERT(params.size() == innerData.numParams_);
        for (int i = 0; i < params.size(); i++)
            RC_ASSERT(params[i] == innerData.params_[i]);

        // Dependent param & type check
        // If it has a type, it's not dependent => dependentIdx == -1
        // It it doesn't have a type, it's dependent => dependentIdx != -1
        for (int i = 0; i < params.size(); i++) {
            if (innerData.types_[i]) {
                RC_ASSERT(innerData.dependentIndices_[i] == -1);
            } else {
                RC_ASSERT(innerData.dependentIndices_[i] >= 0);
                RC_ASSERT(innerData.dependentIndices_[i] < i);
            }
        }
    });
}

TEST_CASE_METHOD(GenGenericParamsFixture, "GenGenericParams.genParams without concepts") {
    types_.init(*this);
    rc::prop("testing property", [=]() {
        GenGenericParams::Options options;
        options.useConcept = false;
        GenGenericParams tested{options};
        auto paramsList = tested.genParameters(Location());

        // Now check the returned parameters

        // No returned param should be concept
        const auto& innerData = tested.innerData();
        for (int i = 0; i < innerData.numParams_; i++) {
            auto t = innerData.types_[i];
            if (t)
                RC_ASSERT(t.kind() != typeKindConcept);
        }

        RC_ASSERT(!tested.usesConcepts());
    });
}

TEST_CASE_METHOD(GenGenericParamsFixture, "GenGenericParams.genParams without CT") {
    types_.init(*this);
    rc::prop("all returned types must be RT or Concept", [=]() {
        GenGenericParams::Options options;
        options.useCt = false;
        GenGenericParams tested{options};
        auto paramsList = tested.genParameters(Location());

        // Now check the returned parameters

        // No returned param should be concept
        const auto& innerData = tested.innerData();
        for (int i = 0; i < innerData.numParams_; i++) {
            auto t = innerData.types_[i];
            if (t)
                RC_ASSERT(t.mode() == modeRt || t.kind() == typeKindConcept);
        }

        RC_ASSERT(!tested.hasCtParams());
    });
}

TEST_CASE_METHOD(GenGenericParamsFixture, "GenGenericParams.genParams without RT") {
    types_.init(*this);
    rc::prop("all returned types must be CT or Concept", [=]() {
        GenGenericParams::Options options;
        options.useRt = false;
        GenGenericParams tested{options};
        auto paramsList = tested.genParameters(Location());

        // Now check the returned parameters

        // No returned param should be concept
        const auto& innerData = tested.innerData();
        for (int i = 0; i < innerData.numParams_; i++) {
            auto t = innerData.types_[i];
            if (t)
                RC_ASSERT(t.mode() == modeCt || t.kind() == typeKindConcept);
        }
    });
}

TEST_CASE_METHOD(GenGenericParamsFixture, "GenGenericParams.genParams without dependent") {
    types_.init(*this);
    rc::prop("all returned params must have types", [=]() {
        GenGenericParams::Options options;
        options.useDependent = false;
        GenGenericParams tested{options};
        auto paramsList = tested.genParameters(Location());

        // Now check the returned parameters

        // No returned param should be concept
        const auto& innerData = tested.innerData();
        for (int i = 0; i < innerData.numParams_; i++) {
            auto t = innerData.types_[i];
            RC_ASSERT(t);
        }

        // We can compute the types of the params, and we need to have types for them
        paramsList.setContext(globalContext_);
        RC_ASSERT(paramsList.computeType());
        auto params = NodeRangeT<ParameterDecl>(paramsList.children());
        for (auto p : params) {
            RC_ASSERT(p.type());
        }

        RC_ASSERT(!tested.hasDepedentParams());
    });
}

TEST_CASE_METHOD(GenGenericParamsFixture, "GenGenericParams.genBoundValues") {
    types_.init(*this, SampleTypes::onlyNumeric | SampleTypes::addByteType);
    rc::prop("bound values correspond to the parameter types", [=]() {
        GenGenericParams::Options options;
        options.useDependent = false;
        GenGenericParams tested{options};
        auto paramsList = tested.genParameters(Location());
        auto values = tested.genBoundValues(Location(), globalContext_, types_);
        const auto& innerData = tested.innerData();

        // Now check the returned parameters

        // The number of values must correspond to the number of parameters
        RC_ASSERT(int(values.size()) == innerData.numParams_);

        // All the values, must have their type computed
        for (auto v : values) {
            if (v)
                RC_ASSERT(v.type());
        }

        // For each param...
        for (int i = 0; i < innerData.numParams_; i++) {
            auto t = innerData.types_[i];
            RC_LOG() << "param type: " << t << "; value: " << values[i] << endl;

            // Dependent param => bound value is a type
            if (!t) {
                RC_ASSERT(values[i].type() == StdDef::typeType);
            }

            // Concept type => bound value is a type
            else if (t.kind() == typeKindConcept) {
                RC_ASSERT(values[i].type() == StdDef::typeType);
            }

            // RT param => NO bound value
            else if (t.mode() == modeRt) {
                RC_ASSERT(values[i] == NodeHandle());
            }

            // CT param => bound value matches type
            else if (t.mode() == modeCt) {
                RC_ASSERT(t == values[i].type());
            }

            // No other case
            else {
                RC_ASSERT(false);
            }
        }
    });
}

// TEST_CASE_METHOD(GenGenericParamsFixture, "GenGenericParams.genParams dependent params check") {
//     types_.init(*this);
//     rc::prop("all depedent params are types", [=]() {
//         GenGenericParams::Options options;
//         options.useDependent = false;
//         GenGenericParams tested{options};
//         auto paramsList = tested.genParameters(Location());

//         // Now check the returned parameters

//         // No returned param should be concept
//         const auto& innerData = tested.innerData();
//         for (int i = 0; i < innerData.numParams_; i++) {
//             auto t = innerData.types_[i];
//             RC_ASSERT(t);
//         }

//         // We can compute the types of the params, and we need to have types for them
//         paramsList.setContext(globalContext_);
//         RC_ASSERT(paramsList.computeType());
//         auto params = NodeRangeT<ParameterDecl>(paramsList.children());
//         for (auto p : params) {
//             RC_ASSERT(p.type());
//         }

//         RC_ASSERT(!tested.hasDepedentParams());
//     });
// }