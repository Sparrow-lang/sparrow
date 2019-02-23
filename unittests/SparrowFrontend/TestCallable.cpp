#include "StdInc.h"
#include "Common/RcBasic.hpp"
#include "Common/TypeFactory.hpp"
#include "Common/GeneralFixture.hpp"
#include "Common/FeatherNodeFactory.hpp"
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
#include "SparrowFrontend/Helpers/StdDef.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Services/ICallableService.h"
#include "SparrowFrontend/Services/IConceptsService.h"
#include "SparrowFrontend/Services/Callable/Callable.h"
#include "SparrowFrontend/Services/Callable/ConceptCallable.h"
#include "SparrowFrontend/Services/Callable/GenericPackageCallable.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

struct CallableFixture : SparrowGeneralFixture {
    CallableFixture();
    ~CallableFixture();

    //! Ensures that the generated callable matches the original decl
    void checkCallable(Callable& c, NodeHandle decl, const ParamsData* paramsData = nullptr);

    //! The types that we are using while performing our tests
    SampleTypes types_;
};

CallableFixture::CallableFixture() {}

CallableFixture::~CallableFixture() {}

void CallableFixture::checkCallable(Callable& c, NodeHandle decl, const ParamsData* paramsData) {
    RC_ASSERT(c.valid());
    RC_ASSERT(c.decl());

    if (decl.kind() == nkSparrowDeclSprFunction) {
        RC_ASSERT(c.decl().kind() == nkFeatherDeclFunction ||
                  c.decl().kind() == nkSparrowDeclGenericFunction);
        bool isGeneric = c.decl().kind() == nkSparrowDeclGenericFunction;
        if (paramsData)
            RC_ASSERT(paramsData->isGeneric() == isGeneric);
    } else if (decl.kind() == nkSparrowDeclSprDatatype) {
        RC_ASSERT(c.decl().kind() == nkSparrowDeclGenericDatatype);
        if (paramsData)
            RC_ASSERT(paramsData->isGeneric());
    } else if (decl.kind() == nkSparrowDeclPackage) {
        RC_ASSERT(c.decl().kind() == nkSparrowDeclGenericPackage);
        if (paramsData)
            RC_ASSERT(paramsData->isGeneric());
    } else if (decl.kind() == nkSparrowDeclSprConcept) {
        RC_ASSERT(c.decl() == decl);
    }

    // Check params
    if (paramsData)
        RC_ASSERT(c.numParams() == paramsData->numParams_);
}

namespace {
SprFunctionDecl genTypeOfFunction(const Location& loc) {
    NodeList params = NodeList::create(loc, NodeRange{}, true);
    NodeHandle returnType = StdDef::clsType;
    NodeHandle body;
    NodeHandle ifClause;

    return SprFunctionDecl::create(loc, "typeOf", params, returnType, body, ifClause);
}
} // namespace

TEST_CASE_METHOD(CallableFixture, "CallableFixture.getCallables") {

    types_.init(*this, SampleTypes::addByteType);

    rc::prop("calling getCallables for functions return results", [=]() {
        ParamsData paramsData = *arbParamsData();
        auto decl = *arbFunction(paramsData);
        if (!decl)
            return;
        decl.setContext(globalContext_);
        FeatherNodeFactory::instance().setContextForAuxNodes(globalContext_);

        Callables res = g_CallableService->getCallables(NodeRange{decl}, modeRt);
        RC_ASSERT(res.size() == 1);
        checkCallable(*res[0], decl, &paramsData);
    });

    rc::prop("calling getCallables for any callable decls return results", [=]() {
        auto decl = *arbCallableDecl();
        if (!decl)
            return;
        decl.setContext(globalContext_);
        FeatherNodeFactory::instance().setContextForAuxNodes(globalContext_);

        Callables res = g_CallableService->getCallables(NodeRange{decl}, modeRt);
        RC_ASSERT(res.size() == 1);
        checkCallable(*res[0], decl);
    });
}

TEST_CASE_METHOD(CallableFixture, "CallableFixture.canCall") {

    types_.init(*this, SampleTypes::addByteType);

    // Add a 'typeOf' function
    auto typeOfFun = genTypeOfFunction(createLocation());
    typeOfFun.setContext(globalContext_);

    rc::prop("calling canCall with valid arguments yields success", [=]() {
        ParamsGenOptions options;
        // options.useConcept = false;
        ParamsData paramsData = *arbParamsData(options);
        auto decl = *arbFunction(paramsData);
        if (!decl)
            return;

        // Generate arguments that we can use to 'call' the decl
        auto args = *arbArguments(paramsData, &types_);
        decl.setContext(globalContext_);
        semanticCheck(args, globalContext_);

        // Generate a callable to test with
        Callables callables = g_CallableService->getCallables(NodeRange{decl}, modeRt);
        RC_ASSERT(callables.size() == 1);
        Callable& c = *callables[0];

        // Ensure we can call the generic
        auto cvt = c.canCall(CCLoc{globalContext_, createLocation()}, args, modeRt, allowCustomCvt);
        if (!cvt) {
            printNode(decl);
            cout << args << endl;
            cout << args[0].type() << endl;
            RC_ASSERT(cvt != convNone);
        }
    });

    rc::prop("calling generateCall works ok, if arguments match exactly", [=]() {
        ParamsGenOptions options;
        // options.useConcept = false;
        ParamsData paramsData = *arbParamsData(options);
        auto decl = *arbFunction(paramsData);
        if (!decl)
            return;

        // Generate arguments that we can use to 'call' the decl
        auto args = *arbArguments(paramsData, &types_, true);
        decl.setContext(globalContext_);
        semanticCheck(args, globalContext_);

        // Generate a callable to test with
        Callables callables = g_CallableService->getCallables(NodeRange{decl}, modeRt);
        RC_ASSERT(callables.size() == 1);
        Callable& c = *callables[0];

        // Ensure we can call the generic
        auto cvt = c.canCall(CCLoc{globalContext_, createLocation()}, args, modeRt, allowCustomCvt);
        RC_ASSERT(cvt != convNone);

        // Generate the call code, and ensure that it semantically checks ok
        // auto callCode = generateCall(c, globalContext_, createLocation());
        // callCode.setContext(globalContext_);
        // RC_ASSERT(callCode.semanticCheck());
        // TODO: this doesn't work right with concepts & references
    });
}

TEST_CASE_METHOD(CallableFixture, "CallableFixture.ConceptCallable") {

    types_.init(*this, SampleTypes::addByteType);

    rc::prop("test ConceptCallable properties", [=]() {
        auto conceptDecl = *arbConcept();
        auto callable = ConceptCallable(conceptDecl);

        RC_ASSERT(callable.decl() == conceptDecl);

        // Generate a random expression
        auto exp = *FeatherNodeFactory::instance().arbExp();
        exp.setContext(globalContext_);
        FeatherNodeFactory::instance().setContextForAuxNodes(globalContext_);

        // We can call the callable with this expression
        CCLoc ccloc{globalContext_, createLocation()};
        auto cvt = callable.canCall(ccloc, NodeRange{exp}, modeRt, CustomCvtMode::allowCustomCvt);
        RC_ASSERT(cvt != convNone);

        RC_ASSERT(callable.valid());
        RC_ASSERT(callable.numParams() == 1);
        RC_ASSERT(callable.paramType(0) == ConceptType::get());

        // Generate the call, and check the result
        auto callCode = callable.generateCall(ccloc);
        RC_ASSERT(callCode.semanticCheck());
        auto ctVal = callCode.kindCast<Feather::CtValueExp>();
        RC_ASSERT(ctVal);
        bool conceptFulfilled = g_ConceptsService->conceptIsFulfilled(conceptDecl, exp.type());
        RC_ASSERT(ctVal.valueDataT<bool>() == conceptFulfilled);
    });
}

namespace {
GenericPackageCallable genGenericPackageCallable(
        CompilationContext* ctx, ParamsData& outParamsData, bool ifClauseVal = true) {
    ParamsGenOptions paramOptions;
    paramOptions.minNumParams = 1;
    paramOptions.useRt = false;
    paramOptions.useConcept = false;
    paramOptions.useDependent = false;
    outParamsData = *arbParamsData(paramOptions);

    // Get an arbitrary GenericPackage decl node
    auto packageDecl = *arbGenPackage(outParamsData, ifClauseVal);
    RC_ASSERT(packageDecl.parameters().children().size() > 0);
    packageDecl.setContext(ctx);
    FeatherNodeFactory::instance().setContextForAuxNodes(ctx);
    RC_ASSERT(packageDecl.semanticCheck());
    auto genPackageDecl = NodeHandle(resultingDecl(packageDecl)).kindCast<GenericPackage>();
    RC_ASSERT(genPackageDecl);

    return GenericPackageCallable(genPackageDecl);
}
} // namespace

TEST_CASE_METHOD(CallableFixture, "CallableFixture.GenericPackageCallable") {

    types_.init(*this, SampleTypes::addByteType);

    rc::prop("test GenericPackageCallable properties", [=]() {
        ParamsData paramsData;
        GenericPackageCallable callable = genGenericPackageCallable(globalContext_, paramsData);

        // Ensure that all the types of the callable match the params data
        RC_ASSERT(callable.numParams() == paramsData.numParams_);
        for (int i = 0; i < paramsData.numParams_; i++) {
            RC_ASSERT(callable.paramType(i) == paramsData.types_[i]);
        }

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        // Check that we can call the callable with the given args
        CCLoc ccloc{globalContext_, createLocation()};
        auto cvt = callable.canCall(ccloc, args, modeRt, CustomCvtMode::allowCustomCvt, true);
        RC_ASSERT(cvt != convNone);
        RC_ASSERT(callable.valid());

        // Generate the call, and check the result
        // This generated a DeclExp pointing to a regular, non-generic package
        auto callCode = callable.generateCall(ccloc);
        RC_ASSERT(callCode);
        RC_ASSERT(callCode.semanticCheck());
        auto declExp = callCode.kindCast<DeclExp>();
        RC_ASSERT(declExp);
        RC_ASSERT(declExp.referredDecls().size() == 1);
        auto referredDecl = declExp.referredDecls()[0];
        RC_ASSERT(referredDecl);
        auto instPackage = referredDecl.kindCast<PackageDecl>();
        RC_ASSERT(instPackage);
        RC_ASSERT(instPackage.name() == callable.decl().name());
        RC_ASSERT(!instPackage.parameters());
        RC_ASSERT(!instPackage.ifClause());
    });

    rc::prop("test GenericPackageCallable with false if clause", [=]() {
        ParamsData paramsData;
        GenericPackageCallable callable =
                genGenericPackageCallable(globalContext_, paramsData, false);

        // Ensure that all the types of the callable match the params data
        RC_ASSERT(callable.numParams() == paramsData.numParams_);
        for (int i = 0; i < paramsData.numParams_; i++) {
            RC_ASSERT(callable.paramType(i) == paramsData.types_[i]);
        }

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        // Check that can call results in failure
        CCLoc ccloc{globalContext_, createLocation()};
        auto cvt = callable.canCall(ccloc, args, modeRt, CustomCvtMode::allowCustomCvt);
        RC_ASSERT(cvt == convNone);
        RC_ASSERT(callable.valid());
    });
}
