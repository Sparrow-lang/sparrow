#include "StdInc.h"
#include "GenValueForType.hpp"
#include "SparrowFrontend/SprCommon/GenValueForType.hpp"
#include "SparrowFrontend/SprCommon/SampleTypes.hpp"
#include "Common/FeatherNodeFactory.hpp"
#include "Common/RcBasic.hpp"

#include "SparrowFrontend/SparrowFrontendTypes.hpp"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "Feather/Utils/FeatherUtils.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using rc::Gen;
using namespace rc;
using Nest::NodeHandle;
using Nest::Type;
using Nest::TypeWithStorage;

namespace {
TypeWithStorage getDataTypeWithPtr(Nest::NodeHandle decl, int numReferences, Nest::EvalMode mode) {
    TypeWithStorage res = DataType::get(decl, mode);
    for (int i = 0; i < numReferences; i++)
        res = Feather::PtrType::get(res);
    return res;
}

} // namespace

Gen<NodeHandle> arbValueForType(TypeWithStorage t, const SampleTypes* sampleTypes) {
    return rc::gen::exec([=]() -> NodeHandle {
        // If t is a concept, transform it into a regular type
        TypeWithStorage type = t;
        if (t.kind() == SprFrontend::typeKindConcept) {
            RC_ASSERT(sampleTypes);
            auto compatibleTypes = sampleTypes->typesForConcept(ConceptType(t));
            type = *gen::elementOf(compatibleTypes);
            type = getDataTypeWithPtr(type.referredNode(), t.numReferences(), t.mode());
        }

        RC_ASSERT(type.kind() != SprFrontend::typeKindConcept);
        int weightValueForType = 5;
        int weightCtValue = type.mode() == modeCt && type.numReferences() == 0 ? 7 : 0;
        int weightOtherExp = type.numReferences() > 0 ? 2 : 0;
        return *gen::weightedOneOf<NodeHandle>(
                {{weightValueForType,
                         gen::cast<NodeHandle>(FeatherNodeFactory::instance().arbTypeNode(type))},
                        {weightCtValue,
                                gen::cast<NodeHandle>(
                                        FeatherNodeFactory::instance().arbCtValueExp(type))},
                        {weightOtherExp, FeatherNodeFactory::instance().arbExp(type)}});
    });
}

Gen<NodeHandle> arbValueForTypeIgnoreMode(TypeWithStorage t) {
    RC_ASSERT(t.kind() != SprFrontend::typeKindConcept);
    if (t.mode() == modeRt && t.numReferences() == 0) {
        return rc::gen::exec([t]() -> NodeHandle {
            auto mode = *gen::arbitrary<EvalMode>();
            return *arbValueForType(t.changeMode(mode));
        });

    } else
        return arbValueForType(t);
}

Gen<NodeHandle> arbValueConvertibleTo(TypeWithStorage t, const SampleTypes* sampleTypes) {
    return rc::gen::exec([=]() -> NodeHandle {
        // If t is a concept, transform it into a regular type
        TypeWithStorage type = t;
        if (t.kind() == SprFrontend::typeKindConcept) {
            RC_ASSERT(sampleTypes);
            auto compatibleTypes = sampleTypes->typesForConcept(ConceptType(t));
            type = *gen::elementOf(compatibleTypes);
            type = getDataTypeWithPtr(type.referredNode(), t.numReferences(), t.mode());
        }

        // Get a type that's convertible to our type
        auto tk = type.kind();
        int weightSameType = 1;
        int weightCtType = type.mode() == modeRt && type.numReferences() == 0 ? 1 : 0;
        int weightAddRef = 1;
        int weightFromPlain = tk == typeKindConst ? 1 : 0;
        int weightFromConst = tk == typeKindData ? 1 : 0;
        int weightFromMutable = tk != typeKindTemp ? 1 : 0;
        int weightFromTmp = tk != typeKindMutable ? 1 : 0;

        TypeWithStorage newType;
        auto alt = *gen::weightedElement<int>(
                {{weightSameType, 0}, {weightCtType, 1}, {weightAddRef, 2}, {weightFromPlain, 3},
                        {weightFromConst, 4}, {weightFromMutable, 5}, {weightFromTmp, 6}});
        switch (alt) {
        case 0:
            newType = type;
            break;
        case 1:
            newType = type.changeMode(modeCt);
            break;
        case 2:
            newType = Feather::addRef(type);
            break;
        case 3:
            newType = Feather::removeCategoryIfPresent(type);
            break;
        case 4:
            newType = Feather::ConstType::get(type);
            break;
        case 5:
            newType = Feather::MutableType::get(Feather::removeCategoryIfPresent(type));
            break;
        case 6:
            newType = Feather::TempType::get(Feather::removeCategoryIfPresent(type));
            break;
        }

        // Get a value for the new type
        return *arbValueForType(newType);
    });
}

Gen<NodeHandle> arbBoundValueForType(TypeWithStorage t, const SampleTypes& sampleTypes) {
    if (t.kind() != SprFrontend::typeKindConcept)
        return gen::cast<NodeHandle>(FeatherNodeFactory::instance().arbCtValueExp(t));
    else {
        return rc::gen::exec([&sampleTypes, t]() -> NodeHandle {
            // Get a type that matches the concept
            auto types = sampleTypes.typesForConcept(ConceptType(t));
            TypeWithStorage innerType = types[*rc::gen::inRange(0, int(types.size()))];

            // Ensure it has the same shape as the concept type
            innerType = getDataTypeWithPtr(innerType.referredNode(), t.numReferences(), t.mode());

            // Create a type node for this type
            return SprFrontend::createTypeNode(nullptr, Location(), innerType);
        });
    }
}
