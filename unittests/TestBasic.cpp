#include "StdInc.h"

TEST_CASE("Smoke test") {
    SECTION("a first section") { REQUIRE(true); }
    SECTION("the second section") { REQUIRE_FALSE(false); }
}

TEST_CASE("RapidCheck smoke test") {
    SECTION("example on RapidCheck page") {
        rc::prop("double reversal yields the original value", [](const std::vector<int>& l0) {
            auto l1 = l0;
            std::reverse(begin(l1), end(l1));
            std::reverse(begin(l1), end(l1));
            RC_ASSERT(l0 == l1);
        });
    }
}