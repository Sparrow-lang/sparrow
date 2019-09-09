#include "StdInc.h"
#include "Common/RcBasic.hpp"
#include "Common/TypeFactory.hpp"
#include "Common/GeneralFixture.hpp"
#include "Common/FeatherNodeFactory.hpp"
#include "Common/LocationGen.hpp"
#include "SprCommon/SampleTypes.hpp"
#include "SprCommon/GenGenericParams.hpp"
#include "SprCommon/GenCallableDecl.hpp"
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

struct DeclsFixture : SparrowGeneralFixture {
    //! The types that we are using while performing our tests
    SampleTypes types_;
};

rc::Gen<DataTypeDecl> arbBasicDataTypeDecl() {
    return rc::gen::exec([=]() -> DataTypeDecl {
        auto loc = g_LocationGen();
        int numFields = *gen::inRange(0, 10);
        NodeList body;
        for (int i=0; i<numFields; i++ )
        {
            auto type = *TypeFactory::arbBasicStorageType(modeRt);
            // auto type = *TypeFactory::arbTypeWithStorage(modeRt);
            auto locField = g_LocationGen();
            auto typeNode = TypeNode::create(locField, type);
            auto field = FieldDecl::create(locField, concat("field", i), typeNode);
            body = NodeList::append(body, field);
        }

        return DataTypeDecl::create(loc, "MyDataType", {}, {}, {}, body);
    });
}

int nodeListSize(NodeList nl)
{
    return nl ? nl.children().size() : 0;
}

TEST_CASE_METHOD(DeclsFixture, "DeclsFixture.DataTypeDecl") {
    types_.init(*this);
    rc::prop("compiling DataTypeDecl nodes", [=]() {
        auto datatypeDecl = *arbBasicDataTypeDecl();
        datatypeDecl.setContext(globalContext_);

        // Check that we can compile it
        NodeHandle explanation = datatypeDecl.semanticCheck();
        RC_ASSERT(explanation);
    });

    rc::prop("check StructDecl explanation of a DataTypeDecl", [=]() {
        auto datatypeDecl = *arbBasicDataTypeDecl();
        datatypeDecl.setContext(globalContext_);
        NodeHandle explanation = datatypeDecl.semanticCheck();
        RC_ASSERT(explanation);

        // Check that we expand into a struct decl
        StructDecl structDecl = explanation.kindCast<StructDecl>();
        RC_ASSERT(structDecl);

        // Check that the fields match
        RC_ASSERT(structDecl.fields().size() == nodeListSize(datatypeDecl.body()));
        for ( int i=0; i<structDecl.fields().size(); i++)
        {
            auto structVar = structDecl.fields()[i].kindCast<VarDecl>();
            RC_ASSERT(structVar);
            auto origField = datatypeDecl.body().children()[i].kindCast<FieldDecl>();
            RC_ASSERT(origField);

            // Original field should be explained by the field found in the structure
            RC_ASSERT(origField.explanation() == structVar);

            auto tRes = structVar.type();
            auto tOrig = TypeWithStorage(origField.typeNode().type());

            // No category in the resulting type
            RC_ASSERT(!isCategoryType(tRes));

            // Check for the number of references
            RC_ASSERT(tRes.numReferences() == removeCategoryIfPresent(tOrig).numReferences());

            // If the original type is a cat-type, make sure the category is kept as a property
            if (isCategoryType(tOrig))
                RC_ASSERT(tOrig.kind() == structVar.getCheckPropertyInt(propSprOrigCat));
        }
    });
}
