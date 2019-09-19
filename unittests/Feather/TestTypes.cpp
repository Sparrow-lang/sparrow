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

TEST_CASE_METHOD(FeatherTypesFixture, "VoidType", "[FeatherTypes]") {
    auto voidUnsp = VoidType::get(modeUnspecified);
    auto voidRt = VoidType::get(modeRt);
    auto voidCt = VoidType::get(modeCt);

    SECTION("There are three VoidType instances: rt, ct and unspecified") {
        REQUIRE(voidRt);
        REQUIRE(voidCt);
        REQUIRE(voidUnsp);

        REQUIRE(voidRt != voidCt);
        REQUIRE(voidUnsp != voidRt);
        REQUIRE(voidUnsp != voidCt);

        REQUIRE(voidRt.mode() == modeRt);
        REQUIRE(voidCt.mode() == modeCt);
        REQUIRE(voidUnsp.mode() == modeUnspecified);
    }

    SECTION("VoidType doesn't have storage") {
        REQUIRE(!voidUnsp.hasStorage());
        REQUIRE(!voidRt.hasStorage());
        REQUIRE(!voidCt.hasStorage());
    }

    SECTION("VoidType can be used at RT") {
        REQUIRE(voidUnsp.canBeUsedAtRt());
        REQUIRE(voidRt.canBeUsedAtRt());
        REQUIRE(voidCt.canBeUsedAtRt());
    }

    SECTION("VoidType have their proper kind (sanity)") {
        REQUIRE(voidUnsp.kind() == Feather::VoidType::staticKind());
        REQUIRE(voidRt.kind() == Feather::VoidType::staticKind());
        REQUIRE(voidCt.kind() == Feather::VoidType::staticKind());
    }
}

TEST_CASE_METHOD(FeatherTypesFixture, "Feather storage types", "[FeatherTypes]") {
    using TypeFactory::g_dataTypeDecls;

    SECTION("Feather types usage examples") {
        auto decl = g_dataTypeDecls[3];

        auto tData = DataType::get(decl, modeRt);
        auto tPtr = PtrType::get(tData);
        auto tPtrPtr = PtrType::get(tPtr);

        auto tConst = ConstType::get(tData);
        auto tMut = MutableType::get(tData);
        auto tTemp = TempType::get(tData);

        auto tConstPtr = PtrType::get(tConst);
        auto tMutPtr = PtrType::get(tMut);

        auto tPtrConst = ConstType::get(tPtr);
        auto tConstPtrConst = ConstType::get(tConstPtr);

        auto tConstPtrMut = MutableType::get(tConstPtr);

        REQUIRE(tData.numReferences() == 0);
        REQUIRE(tPtr.numReferences() == 1);
        REQUIRE(tPtrPtr.numReferences() == 2);
        REQUIRE(tConst.numReferences() == 1);
        REQUIRE(tMut.numReferences() == 1);
        REQUIRE(tTemp.numReferences() == 1);
        REQUIRE(tConstPtr.numReferences() == 2);
        REQUIRE(tMutPtr.numReferences() == 2);
        REQUIRE(tPtrConst.numReferences() == 2);
        REQUIRE(tConstPtrConst.numReferences() == 3);
        REQUIRE(tConstPtrMut.numReferences() == 3);
    }

    rc::prop("Can create data types", [](EvalMode mode) {
        NodeHandle decl = g_dataTypeDecls[*rc::gen::inRange(0, (int)g_dataTypeDecls.size())];

        auto type = DataType::get(decl, mode);
        REQUIRE(type);
        REQUIRE(type.kind() == Feather::DataType::staticKind());
        REQUIRE(type.numReferences() == 0);
        REQUIRE(type.referredNode() == decl);
    });
    rc::prop("Can create Ptr types", []() {
        auto baseType = *TypeFactory::arbTypeWithStorage();
        auto type = PtrType::get(baseType);
        REQUIRE(type.numReferences() == baseType.numReferences() + 1);
        REQUIRE(type.referredNode() == baseType.referredNode());

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
        REQUIRE(type.numReferences() == numRefs + 1);
        REQUIRE(type.referredNode() == decl);

        REQUIRE(type.base() == baseType);
    });

    rc::prop("All feather types except Void have storage", []() {
        auto type = *TypeFactory::arbTypeWithStorage();
        REQUIRE(type.hasStorage());
    });

    rc::prop("Type creation respects indicated eval mode", [](EvalMode mode) {
        auto type = *TypeFactory::arbTypeWithStorage(mode);
        REQUIRE(type.mode() == mode);
    });

    // TODO: create a test when the decl makes the types not usable at CT
    rc::prop("Types can be used at RT", []() {
        auto type = *TypeFactory::arbTypeWithStorage();
        REQUIRE(type.canBeUsedAtRt());
    });
}

TEST_CASE_METHOD(FeatherTypesFixture, "Add or remove references", "[FeatherTypes]") {
    rc::prop("Adding reference increases the number of references, and keeps type kind (if "
             "possible)",
            []() {
                TypeWithStorage base = *TypeFactory::arbBasicStorageType();
                auto newType = addRef(base);
                REQUIRE(newType.numReferences() == base.numReferences() + 1);
                auto expectedKind = base.kind() == typeKindData ? typeKindPtr : base.kind();
                REQUIRE(newType.kind() == expectedKind);
            });
    rc::prop("Removing reference decreases the number of references, and keeps type kind (if "
             "possible)",
            []() {
                TypeWithStorage base = *TypeFactory::arbTypeWithRef();
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
        TypeWithStorage base = *TypeFactory::arbTypeWithRef();
        auto newType = removeCatOrRef(base);
        REQUIRE(newType.numReferences() == base.numReferences() - 1);
    });
    rc::prop("Removing all references decreases the number of references to 0", []() {
        TypeWithStorage base = *TypeFactory::arbBasicStorageType();
        auto newType = removeAllRefs(base);
        REQUIRE(newType.numReferences() == 0);
        // Resulting type is always a data type
        REQUIRE(newType.kind() == typeKindData);
    });
    rc::prop("categoryToRefIfPresent does nothing for data types", []() {
        auto base = *TypeFactory::arbDataOrPtrType();
        Type newType = categoryToRefIfPresent(base);
        REQUIRE(newType == base);
    });
    rc::prop("categoryToRefIfPresent keeps the same number of references", []() {
        TypeWithStorage base = *TypeFactory::arbBasicStorageType();
        TypeWithStorage newType = TypeWithStorage(categoryToRefIfPresent(base));
        REQUIRE(newType.numReferences() == base.numReferences());
    });
}
