#include "StdInc.h"
#include "Common/GeneralFixture.hpp"
#include "Common/RcBasic.hpp"
#include "Nest/MockType.hpp"

using namespace Nest;

//! Fixture used in testing the Type basic functionality
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct TypesFixture : NestGeneralFixture {
    TypesFixture();
    ~TypesFixture();
};

TypesFixture::TypesFixture() {}
TypesFixture::~TypesFixture() {}

TEST_CASE_METHOD(TypesFixture, "User can register a new type") {
    SECTION("first time register") {
        // Register our type
        MockType::registerType();
        // Create one type of this kind
        auto myType = MockType::get(true, 0, modeRt);
        // Expect to have only one kind registered
        REQUIRE(myType.kind() == 0);
    }

    SECTION("second time register") {
        // Register our type
        MockType::registerType();
        // Create one type of this kind
        auto myType = MockType::get(true, 0, modeRt);
        // Expect to have only one kind registered
        REQUIRE(myType.kind() == 0);
    }

    SECTION("register multiple types") {
        // Register three types -- only keep track of the last one
        MockType::registerType();
        MockType::registerType();
        MockType::registerType();
        // Create one type of the last kind
        auto myType = MockType::get(true, 0, modeRt);
        // Expect to reach the third type-ID (starting with 0)
        REQUIRE(myType.kind() == 2);
    }
}

TEST_CASE_METHOD(TypesFixture, "Type identity -- equivalent types are identical, same pointer") {
    // Register our type
    MockType::registerType();

    SECTION("Two similar types have the same pointer -- static ") {
        auto t1 = MockType::get(true, 0, modeRt);
        auto t2 = MockType::get(true, 0, modeRt);

        REQUIRE(t1 == t2);
    }
    rc::prop("Two similar types have the same pointer -- arbitrary types", [](MockType t1) {
        auto t2 = MockType::get(t1.hasStorage(), t1.numReferences(), t1.mode());
        REQUIRE(t1 == t2);
    });
    rc::prop("Different type pointers imply different parameters", [](MockType t1, MockType t2) {
        if (t1 != t2) {
            bool cmpStorage = t1.hasStorage() != t2.hasStorage();
            bool cmpRefs = t1.numReferences() != t2.numReferences();
            bool cmpMode = t1.mode() != t2.mode();
            bool cmp = cmpStorage || cmpRefs || cmpMode;
            REQUIRE(cmp);
        }
    });
}

TEST_CASE_METHOD(TypesFixture, "User can change the mode of a type") {
    // Register our type
    MockType::registerType();

    rc::prop("Changing the mode of the type", [](MockType t, EvalMode newMode) {
        auto t2 = t.changeMode(newMode);
        REQUIRE(t2.mode() == newMode);

        if (t.mode() == newMode) {
            // If changing to the same mode, there is no change in type
            REQUIRE(t == t2);
        } else {
            // If the mode is different, we obtain a different type
            REQUIRE(t != t2);
        }
    });
}
