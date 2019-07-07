#include "StdInc.h"
#include "Common/GeneralFixture.hpp"
#include "Common/TypeFactory.hpp"
#include "Common/FeatherNodeFactory.hpp"
#include "SparrowFrontend/SprCommon/GenValueForType.hpp"
#include "SparrowFrontend/SprCommon/SampleTypes.hpp"

#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Services/IConvertService.h"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

struct GenValueForTypeFixture : SparrowGeneralFixture {
    void computeType(Nest::NodeHandle node) {
        RC_ASSERT(node);
        node.setContext(globalContext_);
        FeatherNodeFactory::instance().setContextForAuxNodes(globalContext_);
        RC_ASSERT(node.computeType());
    }

    //! The types that we are using while performing our tests
    SampleTypes types_;
};


TEST_CASE_METHOD(GenValueForTypeFixture, "genValueForType functions generate expected results") {
    types_.init(*this);

    rc::prop("generated values match the given type", [this]() {
        // Generate a type to test value generations for
        Nest::TypeWithStorage t = *TypeFactory::arbBasicStorageType();

        // Generate value that match exact
        auto value = *arbValueForType(t);
        computeType(value);
        RC_ASSERT(value.type() == t);

        // Generate value that match type, ignoring mode
        // This is allowed to generate CT values for RT types
        value = *arbValueForTypeIgnoreMode(t);
        computeType(value);
        RC_ASSERT(Nest::sameTypeIgnoreMode(value.type(), t));

        // Generate value convertible to the given type
        // Ensure that we can convert the type of the value to the given type
        value = *arbValueConvertibleTo(t);
        computeType(value);
        auto cvt = g_ConvertService->checkConversion(globalContext_, value.type(), t);
        RC_ASSERT(cvt.conversionType() != convNone);

        // Generate value compatible with bound values
        // Note, we only treat the non-concept case here: a regular CT type
        // Generated values must match the properties:
        //  - they are CT
        //  - the values must match the type
        if (t.mode() == modeCt) {
            value = *arbBoundValueForType(t, types_);
            computeType(value);
            RC_ASSERT(value.type() == t);

            // Because we would test their value, we require all these values to be CtValueExp
            RC_ASSERT(value.kind() == CtValueExp::staticKind());
        }
    });

    types_.init(*this, SampleTypes::addByteType);

    rc::prop("generated bound values for concepts are ok", [this]() {
        // Generate a concept type
        Nest::TypeWithStorage t = *TypeFactory::arbConceptType();

        auto value = *arbBoundValueForType(t, types_);
        computeType(value);
        RC_ASSERT(value.type().mode() == modeCt);

        // The generated value must be a type value, and the inner type must convert to the given
        // concept
        RC_ASSERT(value.kind() == CtValueExp::staticKind());
        RC_ASSERT(value.type() == SprFrontend::StdDef::typeType);
        auto genType = SprFrontend::getType(value);
        auto cvt = g_ConvertService->checkConversion(globalContext_, genType, t);
        RC_ASSERT(cvt.conversionType() != convNone);
    });
}
