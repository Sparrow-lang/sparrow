#include "StdInc.h"
#include "Common/RcBasic.hpp"
#include "Common/TypeFactory.hpp"
#include "Common/GeneralFixture.hpp"
#include "SprCommon/SampleTypes.hpp"
#include "SprCommon/GenGenericParams.hpp"
#include "SprCommon/Utils.hpp"

#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/Nodes/Builder.h"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

struct CallableFixture : SparrowGeneralFixture {
    CallableFixture();
    ~CallableFixture();

    //! The types that we are using while performing our tests
    SampleTypes types_;
};

CallableFixture::CallableFixture() {}

CallableFixture::~CallableFixture() {}

TEST_CASE_METHOD(CallableFixture, "Test Callable.getCallables") {

    types_.init(*this);

    rc::prop("calling getCallables", [=]() {
    });
}

