#include "StdInc.h"
#include "Common/GeneralFixture.hpp"
#include "Common/RcBasic.hpp"
#include "Common/TypeFactory.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Feather/Api/Feather.h"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Nest/MockNode.hpp"

using namespace Nest;
using namespace Feather;

//! Fixture used in testing the Feather types
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct FeatherTypesFixture : SparrowGeneralFixture {
    FeatherTypesFixture();
    ~FeatherTypesFixture();
};

FeatherTypesFixture::FeatherTypesFixture() {
    using TypeFactory::g_dataTypeDecls;
    g_dataTypeDecls.emplace_back(createNativeDatatypeNode(StringRef("i8"), globalContext_));
    g_dataTypeDecls.emplace_back(createNativeDatatypeNode(StringRef("i16"), globalContext_));
    g_dataTypeDecls.emplace_back(createNativeDatatypeNode(StringRef("i32"), globalContext_));
    g_dataTypeDecls.emplace_back(createDatatypeNode(StringRef("FooType"), globalContext_));
    g_dataTypeDecls.emplace_back(createDatatypeNode(StringRef("BarType"), globalContext_));
    g_dataTypeDecls.emplace_back(createDatatypeNode(StringRef("NullType"), globalContext_));
}
FeatherTypesFixture::~FeatherTypesFixture() = default;

TEST_CASE_METHOD(FeatherTypesFixture, "User can create Feather types with given properties") {
    rc::prop("Can create VoidTypes for proper mode", [](EvalMode mode) {
        auto type = VoidType::get(mode);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather::VoidType::staticKind());
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(!type.hasStorage());
    });
    rc::prop("Can create data types", [](EvalMode mode) {
        using TypeFactory::g_dataTypeDecls;

        NodeHandle decl = g_dataTypeDecls[*rc::gen::inRange(0, (int)g_dataTypeDecls.size())];

        auto type = DataType::get(decl, mode);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather::DataType::staticKind());
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(type.hasStorage());
        REQUIRE(type.numReferences() == 0);
        REQUIRE(type.referredNode() == decl);
    });
    rc::prop("Can create Ptr types", [](EvalMode mode) {
        using TypeFactory::g_dataTypeDecls;

        NodeHandle decl = g_dataTypeDecls[*rc::gen::inRange(0, (int)g_dataTypeDecls.size())];
        int numRefs = *rc::gen::inRange(0, 10);

        auto baseType = Feather::getDataTypeWithPtr(decl, numRefs, mode);
        auto type = PtrType::get(baseType);
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(type.hasStorage());
        REQUIRE(type.numReferences() == numRefs + 1);
        REQUIRE(type.referredNode() == decl);

        REQUIRE(type.base() == baseType);
    });
    rc::prop("Can create Const types", [](EvalMode mode) {
        using TypeFactory::g_dataTypeDecls;

        NodeHandle decl = g_dataTypeDecls[*rc::gen::inRange(0, (int)g_dataTypeDecls.size())];
        int numRefs = *rc::gen::inRange(0, 10);

        auto baseType = Feather::getDataTypeWithPtr(decl, numRefs, mode);
        auto type = ConstType::get(baseType);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather::ConstType::staticKind());
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(type.hasStorage());
        REQUIRE(type.numReferences() == numRefs + 1);
        REQUIRE(type.referredNode() == decl);

        REQUIRE(type.base() == baseType);
    });
    rc::prop("Can create Mutable types", [](EvalMode mode) {
        using TypeFactory::g_dataTypeDecls;

        NodeHandle decl = g_dataTypeDecls[*rc::gen::inRange(0, (int)g_dataTypeDecls.size())];
        int numRefs = *rc::gen::inRange(0, 10);

        auto baseType = Feather::getDataTypeWithPtr(decl, numRefs, mode);
        auto type = MutableType::get(baseType);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather::MutableType::staticKind());
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(type.hasStorage());
        REQUIRE(type.numReferences() == numRefs + 1);
        REQUIRE(type.referredNode() == decl);

        REQUIRE(type.base() == baseType);
    });
    rc::prop("Can create Temp types", [](EvalMode mode) {
        using TypeFactory::g_dataTypeDecls;

        NodeHandle decl = g_dataTypeDecls[*rc::gen::inRange(0, (int)g_dataTypeDecls.size())];
        int numRefs = *rc::gen::inRange(0, 10);

        auto baseType = Feather::getDataTypeWithPtr(decl, numRefs, mode);
        auto type = TempType::get(baseType);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather::TempType::staticKind());
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(type.hasStorage());
        REQUIRE(type.numReferences() == numRefs + 1);
        REQUIRE(type.referredNode() == decl);

        REQUIRE(type.base() == baseType);
    });
}

TEST_CASE_METHOD(FeatherTypesFixture, "User can add or remove references") {
    rc::prop("Adding reference increases the number of references, and keeps type kind (if "
             "possible)",
            []() {
                TypeWithStorage base = *TypeFactory::arbBasicStorageType(modeUnspecified, 0, 10);
                auto newType = addRef(base);
                REQUIRE(newType.numReferences() == base.numReferences() + 1);
                auto expectedKind = base.kind() == typeKindData ? typeKindPtr : base.kind();
                REQUIRE(newType.kind() == expectedKind);
            });
    rc::prop("Removing reference decreases the number of references, and keeps type kind (if "
             "possible)",
            []() {
                TypeWithStorage base = *TypeFactory::arbBasicStorageType(modeUnspecified, 1, 10);
                // Ensure we have one "clean" reference to remove
                RC_PRE(!isCategoryType(base) || base.numReferences() > 1);

                auto newType = removeRef(base);
                REQUIRE(newType.numReferences() == base.numReferences() - 1);
                auto expectedKind = base.kind() == typeKindPtr && base.numReferences() == 1
                                            ? typeKindData
                                            : base.kind();
                REQUIRE(newType.kind() == expectedKind);
            });
    rc::prop("Removing reference (old) decreases the number of references", []() {
        TypeWithStorage base = *TypeFactory::arbBasicStorageType(modeUnspecified, 1, 10);
        auto newType = removeCatOrRef(base);
        REQUIRE(newType.numReferences() == base.numReferences() - 1);
        // Resulting type is always a data type
        REQUIRE(newType.kind() == typeKindData);
    });
    rc::prop("Removing all references decreases the number of references to 0", []() {
        TypeWithStorage base = *TypeFactory::arbBasicStorageType(modeUnspecified, 0, 10);
        auto newType = removeAllRefs(base);
        REQUIRE(newType.numReferences() == 0);
        // Resulting type is always a data type
        REQUIRE(newType.kind() == typeKindData);
    });
    rc::prop("categoryToRefIfPresent does nothing for data types", []() {
        auto base = *TypeFactory::arbDataOrPtrType(modeUnspecified, 0, 10);
        Type newType = categoryToRefIfPresent(base);
        REQUIRE(newType == base);
    });
    rc::prop("categoryToRefIfPresent keeps the same number of references", []() {
        TypeWithStorage base = *TypeFactory::arbBasicStorageType(modeUnspecified, 0, 10);
        TypeWithStorage newType = TypeWithStorage(categoryToRefIfPresent(base));
        REQUIRE(newType.numReferences() == base.numReferences());
    });
}
