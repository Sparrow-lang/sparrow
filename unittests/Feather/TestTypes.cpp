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
    g_dataTypeDecls.push_back(createNativeDatatypeNode(StringRef("i8"), globalContext_));
    g_dataTypeDecls.push_back(createNativeDatatypeNode(StringRef("i16"), globalContext_));
    g_dataTypeDecls.push_back(createNativeDatatypeNode(StringRef("i32"), globalContext_));
    g_dataTypeDecls.push_back(createDatatypeNode(StringRef("FooType"), globalContext_));
    g_dataTypeDecls.push_back(createDatatypeNode(StringRef("BarType"), globalContext_));
    g_dataTypeDecls.push_back(createDatatypeNode(StringRef("NullType"), globalContext_));
}
FeatherTypesFixture::~FeatherTypesFixture() {}

TEST_CASE_METHOD(FeatherTypesFixture, "User can create Feather types with given properties") {
    rc::prop("Can create VoidTypes for proper mode", [](EvalMode mode) {
        auto type = VoidType::get(mode);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather_getVoidTypeKind());
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(!type.hasStorage());
    });
    rc::prop("Can create data types", [](EvalMode mode) {
        using TypeFactory::g_dataTypeDecls;

        NodeHandle decl = g_dataTypeDecls[*rc::gen::inRange(0, (int) g_dataTypeDecls.size())];
        int numRefs = *rc::gen::inRange(0, 10);

        auto type = DataType::get(decl, numRefs, mode);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather_getDataTypeKind());
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(type.hasStorage());
        REQUIRE(type.numReferences() == numRefs);
        REQUIRE(type.referredNode() == decl);
    });
    rc::prop("Can create Const types", [](EvalMode mode) {
        using TypeFactory::g_dataTypeDecls;

        NodeHandle decl = g_dataTypeDecls[*rc::gen::inRange(0, (int) g_dataTypeDecls.size())];
        int numRefs = *rc::gen::inRange(0, 10);

        auto baseType = DataType::get(decl, numRefs, mode);
        auto type = ConstType::get(baseType);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather_getConstTypeKind());
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(type.hasStorage());
        REQUIRE(type.numReferences() == numRefs+1);
        REQUIRE(type.referredNode() == decl);

        REQUIRE(type.base() == baseType);
    });
    rc::prop("Can create Mutable types", [](EvalMode mode) {
        using TypeFactory::g_dataTypeDecls;

        NodeHandle decl = g_dataTypeDecls[*rc::gen::inRange(0, (int) g_dataTypeDecls.size())];
        int numRefs = *rc::gen::inRange(0, 10);

        auto baseType = DataType::get(decl, numRefs, mode);
        auto type = MutableType::get(baseType);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather_getMutableTypeKind());
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(type.hasStorage());
        REQUIRE(type.numReferences() == numRefs+1);
        REQUIRE(type.referredNode() == decl);

        REQUIRE(type.base() == baseType);
    });
    rc::prop("Can create Temp types", [](EvalMode mode) {
        using TypeFactory::g_dataTypeDecls;

        NodeHandle decl = g_dataTypeDecls[*rc::gen::inRange(0, (int) g_dataTypeDecls.size())];
        int numRefs = *rc::gen::inRange(0, 10);

        auto baseType = DataType::get(decl, numRefs, mode);
        auto type = TempType::get(baseType);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather_getTempTypeKind());
        REQUIRE(type.mode() == mode);
        REQUIRE(type.canBeUsedAtRt());
        REQUIRE(type.hasStorage());
        REQUIRE(type.numReferences() == numRefs+1);
        REQUIRE(type.referredNode() == decl);

        REQUIRE(type.base() == baseType);
    });
}

TEST_CASE_METHOD(FeatherTypesFixture, "User can add or remove references") {
    rc::prop("Adding reference increases the number of references, and keeps type kind", []() {
        TypeWithStorage base = *TypeFactory::arbBasicStorageType(modeUnspecified, 0, 10);
        auto newType = addRef(base);
        REQUIRE(newType.numReferences() == base.numReferences()+1);
        REQUIRE(newType.kind() == base.kind());
    });
    rc::prop("Removing reference decreases the number of references, and keeps type kind", []() {
        TypeWithStorage base = *TypeFactory::arbBasicStorageType(modeUnspecified, 1, 10);
        // Ensure we have one "clean" reference to remove
        RC_PRE(!isCategoryType(base) || base.numReferences() > 1);

        auto newType = removeRef(base);
        REQUIRE(newType.numReferences() == base.numReferences()-1);
        REQUIRE(newType.kind() == base.kind());
    });
    rc::prop("Removing reference (old) decreases the number of references", []() {
        TypeWithStorage base = *TypeFactory::arbBasicStorageType(modeUnspecified, 1, 10);
        auto newType = removeCatOrRef(base);
        REQUIRE(newType.numReferences() == base.numReferences()-1);
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
        DataType base = *TypeFactory::arbDataType(modeUnspecified, 0, 10);
        Type newType = categoryToRefIfPresent(base);
        REQUIRE(newType == base);
    });
    rc::prop("categoryToRefIfPresent keeps the same number of references", []() {
        TypeWithStorage base = *TypeFactory::arbBasicStorageType(modeUnspecified, 0, 10);
        TypeWithStorage newType = TypeWithStorage(categoryToRefIfPresent(base));
        REQUIRE(newType.numReferences() == base.numReferences());
    });
}
