#include "StdInc.h"
#include "Common/GeneralFixture.hpp"
#include "SparrowFrontend/SprCommon/GenCallableDecl.hpp"
#include "SparrowFrontend/SprCommon/SampleTypes.hpp"

#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "Nest/Api/Compiler.h"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

struct GenCallableDeclFixture : SparrowGeneralFixture {
    GenCallableDeclFixture();
    ~GenCallableDeclFixture();

    static void checkSemanticallyOk(NodeHandle decl);

    //! The types that we are using while performing our tests
    SampleTypes types_;
};

GenCallableDeclFixture::GenCallableDeclFixture() {}

GenCallableDeclFixture::~GenCallableDeclFixture() {}

void GenCallableDeclFixture::checkSemanticallyOk(NodeHandle decl) {
    if (decl && !decl.semanticCheck()) {
        RC_LOG() << decl;
        printNode(decl);
        RC_ASSERT(!"decl failed semantic check");
    }

    // Make sure we compile everything that we need to compile
    Nest_semanticCheckQueuedNodes();
}


TEST_CASE_METHOD(GenCallableDeclFixture, "Generated callable decls are OK") {
    types_.init(*this, SampleTypes::onlyNumeric | SampleTypes::addByteType);
    rc::prop("all generated function decls can be semantically checked", [=]() {
        GenCallableDecl testedObj(createLocation(), globalContext_, types_);
        const auto& paramsGen = testedObj.paramsGenerator();

        checkSemanticallyOk(testedObj.genFunction());
        checkSemanticallyOk(testedObj.genFunction(false));
    });

    rc::prop("all generated package decls can be semantically checked", [=]() {
        GenCallableDecl testedObj(createLocation(), globalContext_, types_);
        const auto& paramsGen = testedObj.paramsGenerator();

        checkSemanticallyOk(testedObj.genGenPackage());
    });

    rc::prop("all generated generic datatype decls can be semantically checked", [=]() {
        GenCallableDecl testedObj(createLocation(), globalContext_, types_);
        const auto& paramsGen = testedObj.paramsGenerator();

        checkSemanticallyOk(testedObj.genGenDatatype());
    });

    rc::prop("all generated concept decls can be semantically checked", [=]() {
        GenCallableDecl testedObj(createLocation(), globalContext_, types_);
        checkSemanticallyOk(testedObj.genConcept());
    });

    rc::prop("all generated decls can be semantically checked", [=]() {
        GenCallableDecl testedObj(createLocation(), globalContext_, types_);
        checkSemanticallyOk(testedObj.genCallableDecl());
    });
}
