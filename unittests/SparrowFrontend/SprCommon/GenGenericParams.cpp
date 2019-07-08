#include "StdInc.h"
#include "SparrowFrontend/SprCommon/GenGenericParams.hpp"
#include "SparrowFrontend/SprCommon/GenValueForType.hpp"
#include "SparrowFrontend/SprCommon/SampleTypes.hpp"
#include "SparrowFrontend/SprCommon/Utils.hpp"
#include "Common/LocationGen.hpp"
#include "Common/FeatherNodeFactory.hpp"

#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "Nest/Utils/cppif/StringRef.hpp"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

//! Generate a new parameter
ParameterDecl genParam(ParamsGenOptions options, ParamsData& paramsData, const Location& loc,
        StringRef name, bool addInit, const SampleTypes* sampleTypes);

//! Generates a type to be used for a parameters
TypeWithStorage genType(ParamsGenOptions options);

bool ParamsData::usesConcepts() const {
    for (int i = 0; i < numParams_; i++) {
        auto t = types_[i];
        if (t && t.kind() == typeKindConcept)
            return true;
    }
    return false;
}

bool ParamsData::hasCtParams() const {
    for (int i = 0; i < numParams_; i++) {
        auto t = types_[i];
        if (t && t.mode() == modeCt && t.kind() != typeKindConcept)
            return true;
    }
    return false;
}

bool ParamsData::hasDepedentParams() const {
    for (int i = 0; i < numParams_; i++) {
        auto t = types_[i];
        if (!t)
            return true;
    }
    return false;
}

bool ParamsData::isGeneric() const {
    return usesConcepts() || hasCtParams() || hasDepedentParams();
}

rc::Gen<ParamsData> arbParamsData(ParamsGenOptions options, const SampleTypes* sampleTypes) {
    return rc::gen::exec([=]() -> ParamsData {
        ParamsData res;
        auto loc = g_LocationGen();

        res.numParams_ = 0;
        res.paramsNode_ = NodeList::create(loc, {}, true);
        int numParams = *gen::inRange(options.minNumParams, maxNumParams);
        int idxStartInit = numParams;
        if (randomChance(45))
            idxStartInit = *gen::inRange(options.minNumParams, numParams+1);
        for (int i = 0; i < numParams; i++) {
            string name = concat("p", i + 1);
            auto param = genParam(options, res, loc, name, i>=idxStartInit, sampleTypes);
            res.paramsNode_.addChild(param);
        }
        res.idxStartInit_ = idxStartInit;
        return res;
    });
}

rc::Gen<vector<NodeHandle>> arbBoundValues(const ParamsData& params, const SampleTypes& types) {
    return rc::gen::exec([=, &types]() -> vector<NodeHandle> {
        vector<NodeHandle> values;
        vector<Type> valTypes;
        values.reserve(params.numParams_);
        valTypes.reserve(params.numParams_);
        for (int i = 0; i < params.numParams_; i++) {
            auto t = params.types_[i];
            NodeHandle value;
            Type valueType = t;

            // If this is a regular param (RT, non-concept), don't create a bound value for it
            if (t && t.kind() != typeKindConcept && t.mode() == modeRt) {
                values.emplace_back(nullptr);
                valTypes.emplace_back(nullptr);
                continue;
            }

            // If this is a dependent param, create a corresponding type-node value
            if (!t) {
                int prevIdx = params.dependentIndices_[i];
                valueType = valTypes[prevIdx];

                // If the previous value points to a concept bound val, extract the stored type
                auto prevVal = values[prevIdx];
                auto prevValAsCtValue = prevVal.kindCast<CtValueExp>();
                if (prevValAsCtValue && prevValAsCtValue.valueType() == StdDef::typeType) {
                    auto* t = Feather_getCtValueData<Type>(prevValAsCtValue);
                    if (!t || !*t)
                        REP_INTERNAL(prevValAsCtValue.location(), "No type was set for node");
                    ASSERT(t);
                    valueType = *t;
                }
                value = createTypeNode(nullptr, g_LocationGen(), valueType);
            } else {
                // Get a normal value for type
                value = *arbBoundValueForType(t, types);
            }

            values.push_back(value);
            valTypes.push_back(valueType);
        }
        return values;
    });
}

rc::Gen<vector<NodeHandle>> arbArguments(const ParamsData& params, const SampleTypes* sampleTypes, bool exactMatch) {
    return rc::gen::exec([=]() -> vector<NodeHandle> {
        vector<NodeHandle> values;
        values.reserve(params.numParams_);
        for (int i = 0; i < params.numParams_; i++) {
            auto t = params.types_[i];

            // If this is a dependent param, get the type we are referring to
            if (!t) {
                int prevIdx = params.dependentIndices_[i];
                while (!params.types_[prevIdx])
                    prevIdx = params.dependentIndices_[prevIdx];
                t = params.types_[prevIdx];
            }

            NodeHandle value;
            if (exactMatch)
                value = *arbValueForType(t, sampleTypes);
            else
                value = *arbValueConvertibleTo(t, sampleTypes);
            values.push_back(value);
        }
        return values;
    });
}

void semanticCheck(const vector<NodeHandle>& values, Nest::CompilationContext* ctx) {
    // Set the context for all the bound values
    for (auto val : values)
        if (val)
            val.setContext(ctx);
    // Also set the context for any other aux nodes
    FeatherNodeFactory::instance().setContextForAuxNodes(ctx);
    // Now do the semantic checking of the bound values
    for (auto val : values)
        if (val)
            RC_ASSERT(val.semanticCheck());
}

ParameterDecl genParam(ParamsGenOptions options, ParamsData& paramsData, const Location& loc,
        StringRef name, bool addInit, const SampleTypes* sampleTypes) {
    int curIdx = paramsData.numParams_;

    // Should we have a dependent param?
    // If yes, pick up a parameter to use
    StringRef prevParamName{};
    bool isDependent = options.useDependent && paramsData.numParams_ > 0 && randomChance(25);
    int dependentIdx = -1;
    if (isDependent) {
        dependentIdx = *gen::inRange(0, curIdx);
        auto t = paramsData.types_[dependentIdx];
        if (t && t.mode() == modeRt) {
            // We cannot be dependent on a RT param; fallback
            isDependent = false;
            dependentIdx = -1;
        } else
            prevParamName = paramsData.params_[dependentIdx].name();
    }

    // If not dependent, generate a type to use
    TypeWithStorage t;
    if (!isDependent)
        t = genType(options);

    // Generate the type node for the parameter
    NodeHandle typeNode;
    if (!isDependent)
        typeNode = createTypeNode(nullptr, loc, t);
    else {
        auto arg = Identifier::create(loc, prevParamName);
        typeNode = FunApplication::create(loc, "typeOf", NodeRange({arg}));
    }

    // Optionally, use an initializer for the parameter
    NodeHandle init;
    if (addInit) {
        if (isDependent)
            init = Identifier::create(loc, prevParamName);
        else if (t.kind() != typeKindConcept || sampleTypes)
            init = *arbValueForType(t, sampleTypes);
    }

    // Finally, create the parameter
    auto res = ParameterDecl::create(loc, name, typeNode, init);

    paramsData.params_[curIdx] = res;
    paramsData.types_[curIdx] = t;
    paramsData.dependentIndices_[curIdx] = dependentIdx;
    paramsData.numParams_++;

    return res;
}

TypeWithStorage genType(ParamsGenOptions options) {
    bool useConcept = options.useConcept && randomChance(30);
    if (useConcept) {
        return *TypeFactory::arbConceptType(modeRt);    // always use RT for concepts
    } else {
        EvalMode mode = modeUnspecified;
        if (!options.useCt)
            mode = modeRt;
        if (!options.useRt)
            mode = modeCt;
        return *TypeFactory::arbDataType(mode);
    }
}
