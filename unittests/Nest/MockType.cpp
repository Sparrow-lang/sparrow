#include "StdInc.h"
#include "MockType.hpp"
#include "Common/RcBasic.hpp"
#include "Nest/Api/Type.h"
#include "Nest/Api/TypeKindRegistrar.h"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.hpp"

int MockType::typeKind = -1;

using namespace Nest;

Nest_TypeRef MockType_ChangeTypeMode(Nest_TypeRef t, Nest_EvalMode mode) {
    return MockType(t).changeMode(mode);
}

void MockType::registerType() { typeKind = Nest_registerTypeKind(&MockType_ChangeTypeMode); }

MockType MockType::get(bool hasStorage, int numRefs, Nest::EvalMode mode) {
    Nest_Type referenceType = {0};
    referenceType.typeKind = MockType::typeKind;
    referenceType.mode = mode;
    referenceType.numSubtypes = 0;
    referenceType.numReferences = numRefs;
    referenceType.hasStorage = hasStorage ? 1 : 0;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags = 0;
    referenceType.referredNode = nullptr;
    referenceType.description = "MockType"; // Dummy for now

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Update the description
        ostringstream oss;
        oss << MockType{&referenceType};
        referenceType.description = dupString(oss.str().c_str());
        // Insert the new type
        t = Nest_insertStockType(&referenceType);
    }
    return {t};
}

MockType::MockType() {}

MockType::MockType(Nest::TypeRef t)
    : type(t) {}

int MockType::kind() const {
    REQUIRE(type->typeKind == typeKind);
    return typeKind;
}

bool MockType::hasStorage() const { return type->hasStorage != 0; }
int MockType::numReferences() const { return type->numReferences; }
Nest::EvalMode MockType::mode() { return type->mode; }

MockType MockType::changeMode(Nest::EvalMode newMode) const {
    return MockType::get(type->hasStorage != 0, type->numReferences, newMode);
}

ostream& operator<<(ostream& os, MockType t) {
    return os << "MockType(" << (t.hasStorage() ? "storage" : "no-storage") << ", refs:" << t.numReferences() << ", "
              << t.mode() << ")";
}

rc::Gen<MockType> arbMockType() {
    using namespace rc;

    auto storageGen = gen::arbitrary<bool>();
    auto numRefsGen = gen::inRange(0, 10);
    auto modeGen = gen::arbitrary<EvalMode>();
    return gen::apply(
            [=](bool hasStorage, int numReferences, EvalMode mode) -> MockType {
                return MockType::get(hasStorage, numReferences, mode);
            },
            storageGen, numRefsGen, modeGen);
}
