#include "StdInc.h"
#include "Common/GeneralFixture.hpp"
#include "Common/FeatherNodeFactory.hpp"
#include "SparrowFrontend/SprCommon/GenGenericParams.hpp"
#include "SparrowFrontend/SprCommon/SampleTypes.hpp"

#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "Nest/Utils/cppif/NodeRange.hpp"

using namespace Nest;
using namespace Feather;
using namespace SprFrontend;
using namespace rc;

struct GenGenericParamsFixture : SparrowGeneralFixture {
    //! The types that we are using while performing our tests
    SampleTypes types_;
};


TEST_CASE_METHOD(GenGenericParamsFixture, "Test GenGenericParams.genParams") {
    types_.init(*this);
    rc::prop("generated parameters match constraints (default options)", [=]() {
        auto res = *arbParamsData();

        // Now check the returned parameters

        auto params = NodeRangeT<ParameterDecl>(res.paramsNode_.children());

        // The parameters match
        RC_ASSERT(params.size() == res.numParams_);
        for (int i = 0; i < params.size(); i++)
            RC_ASSERT(params[i] == res.params_[i]);

        // Dependent param & type check
        // If it has a type, it's not dependent => dependentIdx == -1
        // It it doesn't have a type, it's dependent => dependentIdx != -1
        for (int i = 0; i < params.size(); i++) {
            if (res.types_[i]) {
                RC_ASSERT(res.dependentIndices_[i] == -1);
            } else {
                RC_ASSERT(res.dependentIndices_[i] >= 0);
                RC_ASSERT(res.dependentIndices_[i] < i);
            }
        }
    });

    rc::prop("generated parameters match constraints (without concepts)", [=]() {
        ParamsGenOptions options;
        options.useConcept = false;
        auto res = *arbParamsData(options);

        // Now check the returned parameters

        // No returned param should be concept
        for (int i = 0; i < res.numParams_; i++) {
            auto t = res.types_[i];
            if (t)
                RC_ASSERT(!isConceptType(t));
        }

        RC_ASSERT(!res.usesConcepts());
    });

    rc::prop("params without CT => all returned types must be RT or Concept", [=]() {
        ParamsGenOptions options;
        options.useCt = false;
        auto res = *arbParamsData(options);

        // Now check the returned parameters

        // No returned param should be concept
        for (int i = 0; i < res.numParams_; i++) {
            auto t = res.types_[i];
            if (t)
                RC_ASSERT(t.mode() == modeRt || isConceptType(t));
        }

        RC_ASSERT(!res.hasCtParams());
    });

    rc::prop("without RT => all returned types must be CT or Concept", [=]() {
        ParamsGenOptions options;
        options.useRt = false;
        auto res = *arbParamsData(options);

        // Now check the returned parameters

        for (int i = 0; i < res.numParams_; i++) {
            auto t = res.types_[i];
            if (t)
                RC_ASSERT(t.mode() == modeCt || isConceptType(t));
        }
    });

    rc::prop("without dependent => all returned params must have types", [=]() {
        ParamsGenOptions options;
        options.useDependent = false;
        auto res = *arbParamsData(options);

        // Now check the returned parameters

        // No returned param should be concept
        for (int i = 0; i < res.numParams_; i++) {
            auto t = res.types_[i];
            RC_ASSERT(t);
        }

        // We can compute the types of the params, and we need to have types for them
        res.paramsNode_.setContext(globalContext_);
        RC_ASSERT(res.paramsNode_.computeType());
        auto params = NodeRangeT<ParameterDecl>(res.paramsNode_.children());
        for (auto p : params) {
            RC_ASSERT(p.type());
        }

        RC_ASSERT(!res.hasDepedentParams());
    });

    rc::prop("all depedent params are types", [=]() {
        ParamsGenOptions options;
        options.useDependent = false;
        auto res = *arbParamsData(options);

        // Now check the returned parameters

        // No returned param should be concept
        for (int i = 0; i < res.numParams_; i++) {
            auto t = res.types_[i];
            RC_ASSERT(t);
        }

        // We can compute the types of the params, and we need to have types for them
        res.paramsNode_.setContext(globalContext_);
        RC_ASSERT(res.paramsNode_.computeType());
        auto params = NodeRangeT<ParameterDecl>(res.paramsNode_.children());
        for (auto p : params) {
            RC_ASSERT(p.type());
        }

        RC_ASSERT(!res.hasDepedentParams());
    });

    rc::prop("Generated parameters can be successfully compiled", [=]() {
        ParamsGenOptions options;
        options.useDependent = false;
        auto res = *arbParamsData(options);

        res.paramsNode_.setContext(globalContext_);
        FeatherNodeFactory::instance().setContextForAuxNodes(globalContext_);

        auto resNode = res.paramsNode_.semanticCheck();
        if (!resNode) {
            RC_LOG() << Nest_toStringEx(res.paramsNode_) << endl;
            printNode(res.paramsNode_);
            RC_ASSERT(false);
        }
    });
}

TEST_CASE_METHOD(GenGenericParamsFixture, "GenGenericParams.genBoundValues") {
    types_.init(*this, SampleTypes::onlyNumeric | SampleTypes::addByteType);
    rc::prop("bound values correspond to the parameter types", [=]() {
        ParamsGenOptions options;
        options.useDependent = false;
        auto params = *arbParamsData(options);
        auto values = *arbBoundValues(params, types_);
        semanticCheck(values, globalContext_);

        // Now check the returned bound values

        // The number of values must correspond to the number of parameters
        RC_ASSERT(int(values.size()) == params.numParams_);

        // All the values, must have their type computed
        for (auto v : values) {
            if (v)
                RC_ASSERT(v.type());
        }

        // For each param...
        for (int i = 0; i < params.numParams_; i++) {
            auto t = params.types_[i];
            RC_LOG() << "param type: " << t << "; value: " << values[i] << endl;

            // Dependent param => bound value is a type
            if (!t) {
                RC_ASSERT(values[i].type() == StdDef::typeType);
            }

            // Concept type => bound value is a type
            else if (isConceptType(t)) {
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
