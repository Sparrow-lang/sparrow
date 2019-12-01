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
//! If the given type is a concept type, generate a data type with the same shape.
//! That is, if the original type has const/mut/ptr applied to the concept type, the same wrappers
//! are applied over data type.
//!
//! This should be only called from rc::gen::exec contexts
TypeWithStorage conceptToDataType(TypeWithStorage t, const SampleTypes* sampleTypes) {
    int kind = t.kind();
    if (kind == typeKindData)
        return t;
    else if (kind == typeKindPtr)
        t = Feather::PtrType::get(conceptToDataType(Feather::PtrType(t).base(), sampleTypes));
    else if (kind == typeKindConst)
        t = Feather::ConstType::get(conceptToDataType(Feather::ConstType(t).base(), sampleTypes));
    else if (kind == typeKindMutable)
        t = Feather::MutableType::get(
                conceptToDataType(Feather::MutableType(t).base(), sampleTypes));
    else if (kind == typeKindTemp)
        t = Feather::TempType::get(conceptToDataType(Feather::TempType(t).base(), sampleTypes));
    else if (kind == SprFrontend::typeKindConcept) {
        RC_ASSERT(sampleTypes);
        auto compatibleTypes = sampleTypes->typesForConcept(ConceptType(t));
        auto dataType = *gen::elementOf(compatibleTypes);
        return DataType::get(dataType.referredNode(), t.mode()); // keep mode
    } else
        REP_INTERNAL(NOLOC, "Cannot transform concept type %1% to data type") % t;
    return t;
}

} // namespace

Gen<NodeHandle> arbValueForType(TypeWithStorage t, const SampleTypes* sampleTypes) {
    return rc::gen::exec([=]() -> NodeHandle {
        // If t is a concept, transform it into a regular type
        TypeWithStorage type = t;
        if (SprFrontend::isConceptType(t))
            type = conceptToDataType(type, sampleTypes);

        RC_ASSERT(!SprFrontend::isConceptType(type));
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
    RC_ASSERT(!SprFrontend::isConceptType(t));
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
        if (SprFrontend::isConceptType(t))
            type = conceptToDataType(type, sampleTypes);

        // Get a type that's convertible to our type
        auto tk = type.kind();
        int weightSameType = 1;
        int weightCtType = type.mode() == modeRt && type.numReferences() == 0 ? 1 : 0;
        int weightAddRef = 1;
        int weightAddMutRef = tk == typeKindPtr ? 1 : 0;
        int weightFromPlain = tk == typeKindConst ? 1 : 0;
        int weightFromConst = tk == typeKindData ? 1 : 0;
        int weightFromMutable = tk != typeKindTemp ? 1 : 0;
        int weightFromTmp = tk != typeKindMutable ? 1 : 0;

        TypeWithStorage newType;
        auto alt = *gen::weightedElement<int>({{weightSameType, 0}, {weightCtType, 1},
                {weightAddRef, 2}, {weightAddMutRef, 3}, {weightFromPlain, 4}, {weightFromConst, 5},
                {weightFromMutable, 6}, {weightFromTmp, 7}});
        switch (alt) {
        case 0:
            newType = type;
            break;
        case 1:
            newType = type.changeMode(modeCt);
            break;
        case 2:
            newType = Feather::PtrType::get(type);
            break;
        case 3:
            newType = Feather::PtrType::get(Feather::MutableType::get(type));
            break;
        case 4:
            newType = Feather::removeCategoryIfPresent(type);
            break;
        case 5:
            newType = Feather::ConstType::get(type);
            break;
        case 6:
            newType = Feather::MutableType::get(Feather::removeCategoryIfPresent(type));
            break;
        case 7:
            newType = Feather::TempType::get(Feather::removeCategoryIfPresent(type));
            break;
        }

        // Get a value for the new type
        return *arbValueForType(newType);
    });
}

Gen<NodeHandle> arbBoundValueForType(TypeWithStorage t, const SampleTypes& sampleTypes) {
    if (!SprFrontend::isConceptType(t))
        return gen::cast<NodeHandle>(FeatherNodeFactory::instance().arbCtValueExp(t));
    else {
        return rc::gen::exec([&sampleTypes, t]() -> NodeHandle {
            auto innerType = conceptToDataType(t, &sampleTypes);

            // Create a type node for this type
            return SprFrontend::createTypeNode(nullptr, Location(), innerType);
        });
    }
}
