#include "StdInc.h"
#include "Common/RcBasic.hpp"
#include "Common/TypeFactory.hpp"
#include "Common/GeneralFixture.hpp"
#include "SprCommon/SampleTypes.hpp"

#include "SparrowFrontend/Helpers/Convert.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using namespace Feather;
using namespace SprFrontend;

struct CallableFixture : SparrowGeneralFixture {
    CallableFixture();
    ~CallableFixture();

    //! The types that we are using while performing our tests
    SampleTypes types_;
};

CallableFixture::CallableFixture() { types_.init(*this); }

CallableFixture::~CallableFixture() {}

TEST_CASE("Test Generics.checkCreateGenericFun") {
    // TODO
}

TEST_CASE("TODO: Calling Feather functions") {
    // TODO
}