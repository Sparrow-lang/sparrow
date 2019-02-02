#include "StdInc.h"
#include "GenValueForType.hpp"
#include "SparrowFrontend/SprCommon/GenValueForType.hpp"
#include "SparrowFrontend/SprCommon/SampleTypes.hpp"
#include "Common/FeatherNodeFactory.hpp"
#include "Common/RcBasic.hpp"

#include "SparrowFrontend/Utils/cppif/SparrowFrontendTypes.hpp"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "Feather/Utils/FeatherUtils.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using rc::Gen;
using namespace rc;
using Nest::NodeHandle;
using Nest::Type;
using Nest::TypeWithStorage;

Gen<NodeHandle> arbValueForType(TypeWithStorage t) {
    int weightValueForType = 5;
    int weightCtValue = t.mode() == modeCt && t.numReferences() == 0 ? 7 : 0;
    int weightOtherExp = t.numReferences() > 0 ? 2 : 0;
    return gen::weightedOneOf<NodeHandle>(
            {{weightValueForType,
                     gen::cast<NodeHandle>(FeatherNodeFactory::instance().arbTypeNode(t))},
                    {weightCtValue,
                            gen::cast<NodeHandle>(FeatherNodeFactory::instance().arbCtValueExp(t))},
                    {weightOtherExp, FeatherNodeFactory::instance().arbExp(t)}});
}

Gen<NodeHandle> arbValueForTypeIgnoreMode(TypeWithStorage t) {
    if (t.mode() == modeRt && t.numReferences() == 0) {
        return rc::gen::exec([t]() -> NodeHandle {
            auto mode = *gen::arbitrary<EvalMode>();
            return *arbValueForType(t.changeMode(mode));
        });

    } else
        return arbValueForType(t);
}

Gen<NodeHandle> arbValueConvertibleTo(TypeWithStorage t) {
    return rc::gen::exec([t]() -> NodeHandle {
        // Get a type that's convertible to our type
        auto tk = t.kind();
        int weightSameType = 1;
        int weightCtType = t.mode() == modeRt && t.numReferences() == 0 ? 1 : 0;
        int weightAddRef = 1;
        int weightFromPlain = tk == typeKindConst ? 1 : 0;
        int weightFromConst = tk == typeKindData ? 1 : 0;
        int weightFromMutable = 1;
        int weightFromTmp = 1;

        TypeWithStorage newType;
        auto alt = *gen::weightedElement<int>(
                {{weightSameType, 0}, {weightCtType, 1}, {weightAddRef, 2}, {weightFromPlain, 3},
                        {weightFromConst, 4}, {weightFromMutable, 5}, {weightFromTmp, 6}});
        switch (alt) {
        case 0:
            newType = t;
            break;
        case 1:
            newType = t.changeMode(modeCt);
            break;
        case 2:
            newType = Feather::addRef(t);
            break;
        case 3:
            newType = Feather::removeCategoryIfPresent(t);
            break;
        case 4:
            newType = Feather::ConstType::get(t);
            break;
        case 5:
            newType = Feather::MutableType::get(Feather::removeCategoryIfPresent(t));
            break;
        case 6:
            newType = Feather::TempType::get(Feather::removeCategoryIfPresent(t));
            break;
        }

        // Get a value for the new type
        return *arbValueForType(newType);
    });
}

Gen<NodeHandle> arbBoundValueForType(TypeWithStorage t, SampleTypes& sampleTypes) {
    if (t.kind() != SprFrontend::typeKindConcept)
        return arbValueForType(t);
    else {
        return rc::gen::exec([&sampleTypes, t]() -> NodeHandle {
            // Get a type that matches the concept
            auto types = sampleTypes.typesForConcept(ConceptType(t));
            auto innerType = types[*rc::gen::inRange(0, int(types.size()))];

            // Ensure it has the same shape as the concept type
            innerType =
                    Feather::DataType::get(innerType.referredNode(), t.numReferences(), t.mode());

            // Create a type node for this type
            return SprFrontend::createTypeNode(nullptr, Location(), innerType);
        });
    }
}
