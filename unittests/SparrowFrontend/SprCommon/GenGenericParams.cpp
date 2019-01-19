#include "StdInc.h"
#include "SparrowFrontend/SprCommon/GenGenericParams.hpp"
#include "SparrowFrontend/SprCommon/SampleTypes.hpp"
#include "SparrowFrontend/SprCommon/Utils.hpp"

#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "Nest/Utils/cppif/StringRef.hpp"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

GenGenericParams::GenGenericParams(Options options)
    : options_(std::move(options)) {}

NodeList GenGenericParams::genParameters(const Location& loc) {
    data_.numParams_ = 0;

    NodeList params = NodeList::create(loc, {}, true);
    int numParams = *gen::inRange(0, maxNumParams);
    for (int i = 0; i < numParams; i++) {
        string name = concat("p", i + 1);
        auto param = genParam(loc, name);
        params.addChild(param);
    }

    return params;
}

vector<NodeHandle> GenGenericParams::genBoundValues(const Location& loc, CompilationContext* ctx, const SampleTypes& types) {
    vector<NodeHandle> values;
    values.reserve(data_.numParams_);
    for (int i = 0; i < data_.numParams_; i++) {
        auto t = data_.types_[i];
        NodeHandle value;

        // If this is a regular param (RT, non-concept), don't create a bound value for it
        if (t && t.kind() != typeKindConcept && t.mode() == modeRt) {
            values.push_back(nullptr);
            continue;
        }

        // If this is a dependent param, create a corresponding type-node value
        if (!t) {
            int prevIdx = data_.dependentIndices_[i];
            auto prevVal = values[prevIdx];

            value = createTypeNode(ctx, loc, prevVal.type());


            int paramIdx = i;
            RC_ASSERT(data_.dependentIndices_[paramIdx] >= 0);
            while (data_.dependentIndices_[paramIdx] >= 0) {
                paramIdx = data_.dependentIndices_[paramIdx];
            }
            t = data_.types_[paramIdx];
            RC_ASSERT(t);
            RC_ASSERT(t.kind() == typeKindConcept || t.mode() == modeCt);
        }
        else {
            // Get a normal value for type
            value = genValueForType(loc, t, types);
        }

        // Ensure that these values are semantically checked
        value.setContext(ctx);
        RC_ASSERT(value.semanticCheck());

        values.push_back(value);
    }
    return values;
}


bool GenGenericParams::usesConcepts() const {
    for (int i = 0; i < data_.numParams_; i++) {
        auto t = data_.types_[i];
        if (t && t.kind() == typeKindConcept)
            return true;
    }
    return false;
}

bool GenGenericParams::hasCtParams() const {
    for (int i = 0; i < data_.numParams_; i++) {
        auto t = data_.types_[i];
        if (t && t.mode() == modeCt && t.kind() != typeKindConcept)
            return true;
    }
    return false;
}

bool GenGenericParams::hasDepedentParams() const {
    for (int i = 0; i < data_.numParams_; i++) {
        auto t = data_.types_[i];
        if (!t)
            return true;
    }
    return false;
}

ParameterDecl GenGenericParams::genParam(const Location& loc, StringRef name) {
    int curIdx = data_.numParams_;

    // Should we have a dependent param?
    // If yes, pick up a parameter to use
    StringRef prevParamName{};
    bool isDependent = options_.useDependent && data_.numParams_ > 0 && randomChance(25);
    int dependentIdx = -1;
    if (isDependent) {
        dependentIdx = *gen::inRange(0, curIdx);
        auto t = data_.types_[dependentIdx];
        if (t && t.mode() == modeRt) {
            // We cannot be dependent on a RT param; fallback
            isDependent = false;
            dependentIdx = -1;
        } else
            prevParamName = data_.params_[dependentIdx].name();
    }

    // If not dependent, generate a type to use
    TypeWithStorage t;
    if (!isDependent)
        t = genType();

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
    if (randomChance(30)) {
        if (!isDependent)
            init = FunApplication::create(loc, createTypeNode(nullptr, loc, t), NodeRange());
        else
            init = Identifier::create(loc, prevParamName);
    }

    // Finally, create the parameter
    auto res = ParameterDecl::create(loc, name, typeNode, init);

    data_.params_[curIdx] = res;
    data_.types_[curIdx] = t;
    data_.dependentIndices_[curIdx] = dependentIdx;
    data_.numParams_++;

    return res;
}

TypeWithStorage GenGenericParams::genType() const {
    bool useConcept = options_.useConcept && randomChance(30);
    if (useConcept) {
        return *TypeFactory::arbConceptType();
    } else {
        EvalMode mode = modeUnspecified;
        if (!options_.useCt)
            mode = modeRt;
        if (!options_.useRt)
            mode = modeCt;
        return *TypeFactory::arbDataType(mode);
    }
}

NodeHandle GenGenericParams::genValueForType(Location loc, TypeWithStorage t, const SampleTypes& types) {
    // For concepts, we generate a type of the given concept
    if (t.kind() == typeKindConcept) {
        // Create a type node containing a type fulfilling the concept
        auto tinst = *rc::gen::element(types.i8Type_, types.i16Type_, types.i32Type_);
        return createTypeNode(nullptr, loc, tinst);
    }

    // For non-concepts, generate a CT value of the given type
    RC_ASSERT(t.kind() == typeKindData);
    auto decl = t.referredNode();
    auto nativeName = decl.getCheckPropertyString(propNativeName);
    RC_ASSERT(*nativeName.begin == 'i');
    nativeName.begin++;
    int numBits = atoi(nativeName.begin);
    RC_ASSERT(numBits > 0);
    RC_ASSERT((numBits % 8) == 0);

    // Get the data for the type
    // We only generate 10 values for each type
    int size = numBits/8;
    int coreVal = *rc::gen::inRange(0, 10);
    char ch = char('0' + coreVal);
    char buf[] = {ch, ch, ch, ch, 0};
    buf[size] = 0;
    StringRef data(buf, buf + size);

    return Feather::CtValueExp::create(loc, t, data);
}

