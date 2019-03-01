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
#include "SparrowFrontend/Services/Callable/GenericDatatypeCallable.h"
#include "SparrowFrontend/Services/Callable/FunctionCallable.h"
#include "SparrowFrontend/Services/Callable/GenericFunctionCallable.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using namespace Feather;
using namespace SprFrontend;
using namespace rc;

struct CallableFixture : SparrowGeneralFixture {
    CallableFixture();
    ~CallableFixture();

    //! Ensures that the generated callable matches the original decl
    void checkCallable(Callable& c, NodeHandle decl, const ParamsData* paramsData = nullptr);

    GenericPackageCallable genGenericPackageCallable(
            ParamsData& outParamsData, bool ifClauseVal = true);
    GenericDatatypeCallable genGenericDatatypeCallable(
            ParamsData& outParamsData, bool ifClauseVal = true);
    FunctionCallable genFunctionCallable(ParamsData& outParamsData);
    GenericFunctionCallable genGenericFunctionCallable(
            ParamsData& outParamsData, bool ifClauseVal = true);

    static void checkCallableParams(Callable& callable, ParamsData& paramsData);

    void checkCanCall(Callable& callable, NodeRange args);
    void checkCannotCall(Callable& callable, NodeRange args);
    void checkGenerateCall(GenericPackageCallable& callable);
    void checkGenerateCall(GenericDatatypeCallable& callable);
    void checkGenerateCall(FunctionCallable& callable);
    void checkGenerateCall(GenericFunctionCallable& callable);

    GenericFunctionCallable getCallable(SprFunctionDecl funDecl);
    void testFunctionCallable(Range<TypeWithStorage> paramTypes, Range<TypeWithStorage> argTypes);

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
        ParamsData paramsData = *arbParamsData({}, &types_);
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
        CCLoc ccloc{globalContext_, createLocation()};
        auto cvt = c.canCall(ccloc, args, modeRt, allowCustomCvt);
        RC_ASSERT(cvt != convNone);

        // Generate the call code, and ensure that it semantically checks ok
        auto callCode = c.generateCall(ccloc);
        callCode.setContext(globalContext_);
        RC_ASSERT(callCode.semanticCheck());
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

GenericPackageCallable CallableFixture::genGenericPackageCallable(
        ParamsData& outParamsData, bool ifClauseVal) {
    ParamsGenOptions paramOptions;
    paramOptions.minNumParams = 1;
    paramOptions.useRt = false;
    paramOptions.useConcept = false;
    paramOptions.useDependent = false;
    outParamsData = *arbParamsData(paramOptions);

    // Get an arbitrary GenericPackage decl node
    auto packageDecl = *arbGenPackage(outParamsData, ifClauseVal);
    // printNode(packageDecl);
    RC_ASSERT(packageDecl.parameters().children().size() > 0);
    packageDecl.setContext(globalContext_);
    FeatherNodeFactory::instance().setContextForAuxNodes(globalContext_);
    RC_ASSERT(packageDecl.semanticCheck());
    auto genPackageDecl = NodeHandle(resultingDecl(packageDecl)).kindCast<GenericPackage>();
    RC_ASSERT(genPackageDecl);

    auto callable = GenericPackageCallable(genPackageDecl);
    checkCallableParams(callable, outParamsData);
    return callable;
}

GenericDatatypeCallable CallableFixture::genGenericDatatypeCallable(
        ParamsData& outParamsData, bool ifClauseVal) {
    ParamsGenOptions paramOptions;
    paramOptions.minNumParams = 1;
    paramOptions.useRt = false;
    paramOptions.useConcept = false;
    paramOptions.useDependent = false;
    outParamsData = *arbParamsData(paramOptions);

    // Get an arbitrary GenericDatatype decl node
    auto datatypeDecl = *arbGenDatatype(outParamsData, ifClauseVal);
    // printNode(datatypeDecl);
    RC_ASSERT(datatypeDecl.parameters().children().size() > 0);
    datatypeDecl.setContext(globalContext_);
    FeatherNodeFactory::instance().setContextForAuxNodes(globalContext_);
    RC_ASSERT(datatypeDecl.semanticCheck());
    auto genDatatypeDecl = NodeHandle(resultingDecl(datatypeDecl)).kindCast<GenericDatatype>();
    RC_ASSERT(genDatatypeDecl);

    auto callable = GenericDatatypeCallable(genDatatypeDecl);
    checkCallableParams(callable, outParamsData);
    return callable;
}

FunctionCallable CallableFixture::genFunctionCallable(ParamsData& outParamsData) {
    ParamsGenOptions paramOptions;
    paramOptions.useCt = true;
    paramOptions.useConcept = false;
    paramOptions.useDependent = false;
    outParamsData = *arbParamsData(paramOptions);

    // Get an arbitrary FunctionDecl node
    auto funDecl = *arbFeatherFunction(outParamsData);
    funDecl.setContext(globalContext_);
    FeatherNodeFactory::instance().setContextForAuxNodes(globalContext_);
    RC_ASSERT(funDecl.semanticCheck());

    auto callable = FunctionCallable(funDecl);
    checkCallableParams(callable, outParamsData);
    return callable;
}

GenericFunctionCallable CallableFixture::genGenericFunctionCallable(
        ParamsData& outParamsData, bool ifClauseVal) {
    ParamsGenOptions paramOptions;
    paramOptions.minNumParams = 1;
    outParamsData = *arbParamsData(paramOptions, &types_);
    RC_PRE(outParamsData.isGeneric());

    // Get an arbitrary GenericFunction decl node
    auto funDecl = *arbFunction(outParamsData, ifClauseVal);
    // printNode(funDecl);
    auto callable = getCallable(funDecl);
    checkCallableParams(callable, outParamsData);
    return callable;
}

void CallableFixture::checkCallableParams(Callable& callable, ParamsData& paramsData) {
    RC_ASSERT(callable.numParams() == paramsData.numParams_);
    for (int i = 0; i < paramsData.numParams_; i++) {
        RC_ASSERT(callable.paramType(i) == paramsData.types_[i]);
    }
}

void CallableFixture::checkCanCall(Callable& callable, NodeRange args) {
    // Check that we can call the callable with the given args
    CCLoc ccloc{globalContext_, createLocation()};
    auto cvt = callable.canCall(ccloc, args, modeRt, CustomCvtMode::allowCustomCvt, true);
    RC_ASSERT(cvt != convNone);
    RC_ASSERT(callable.valid());
}

void CallableFixture::checkCannotCall(Callable& callable, NodeRange args) {
    // Check that we can call the callable with the given args
    CCLoc ccloc{globalContext_, createLocation()};
    auto cvt = callable.canCall(ccloc, args, modeRt, CustomCvtMode::allowCustomCvt);
    RC_ASSERT(cvt == convNone);
    RC_ASSERT(callable.valid());
}

void CallableFixture::checkGenerateCall(GenericPackageCallable& callable) {
    // Generate the call
    CCLoc ccloc{globalContext_, createLocation()};
    auto callCode = callable.generateCall(ccloc);
    RC_ASSERT(callCode);
    RC_ASSERT(callCode.semanticCheck());

    // This generated a DeclExp pointing to a regular, non-generic package
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

    // Check description given to the package
    StringRef desc = instPackage.getCheckPropertyString(propDescription);
    string descStr = desc.toStd();
    RC_ASSERT(desc != instPackage.name());
    RC_ASSERT(descStr.find('[') > 0);
    RC_ASSERT(descStr.find(']') > 0);
}

void CallableFixture::checkGenerateCall(GenericDatatypeCallable& callable) {
    // Generate the call
    CCLoc ccloc{globalContext_, createLocation()};
    auto callCode = callable.generateCall(ccloc);
    RC_ASSERT(callCode);
    RC_ASSERT(callCode.semanticCheck());

    // We have a type node pointing to structure decl corresponding to the datatype
    auto ctVal = callCode.kindCast<CtValueExp>();
    RC_ASSERT(ctVal);
    RC_ASSERT(ctVal.type() == StdDef::typeType);
    auto t = getType(ctVal);
    auto structDecl = DeclNode(t.referredNode());
    RC_ASSERT(t.kind() == typeKindData);
    RC_ASSERT(structDecl.name() == callable.decl().name());

    // The actual type name contains the values. i.e., 'Tuple[Int, Double]''
    StringRef typeDesc = t.description();
    string typeDescStr = typeDesc.toStd();
    RC_ASSERT(typeDesc != structDecl.name());
    RC_ASSERT(typeDescStr.find('[') > 0);
    RC_ASSERT(typeDescStr.find(']') > 0);
}

void CallableFixture::checkGenerateCall(FunctionCallable& callable) {
    // Generate the call
    CCLoc ccloc{globalContext_, createLocation()};
    auto callCode = callable.generateCall(ccloc);
    RC_ASSERT(callCode);
    RC_ASSERT(callCode.semanticCheck());

    // Ensure we have a function call to a function with the same name
    auto funCall = callCode.kindCast<Feather::FunCallExp>();
    if (!funCall) {
        // We are probably creating a temporary variable for returned value
        auto nl = callCode.kindCast<Feather::NodeList>();
        RC_ASSERT(nl);
        RC_ASSERT(nl.children().size() >= 3);
        funCall = nl.children()[1].kindCast<Feather::FunCallExp>();
    }
    RC_ASSERT(funCall);
    auto referredFun = funCall.funDecl();
    RC_ASSERT(referredFun.name() == callable.decl().name());

    // Ensure that the number of arguments is less or equal than the generic callables
    // (the CT parameters will not generate final params)
    RC_ASSERT(funCall.arguments().size() <= callable.numParams());
}

void CallableFixture::checkGenerateCall(GenericFunctionCallable& callable) {
    // Generate the call
    CCLoc ccloc{globalContext_, createLocation()};
    auto callCode = callable.generateCall(ccloc);
    RC_ASSERT(callCode);
    RC_ASSERT(callCode.semanticCheck());

    // Ensure we have a function call to a function with the same name
    auto funCall = callCode.kindCast<Feather::FunCallExp>();
    if (!funCall) {
        // We are probably creating a temporary variable for returned value
        auto nl = callCode.kindCast<Feather::NodeList>();
        RC_ASSERT(nl);
        RC_ASSERT(nl.children().size() >= 3);
        funCall = nl.children()[1].kindCast<Feather::FunCallExp>();
    }
    RC_ASSERT(funCall);
    auto referredFun = funCall.funDecl();
    RC_ASSERT(referredFun.name() == callable.decl().name());

    // Ensure that the number of arguments is less or equal than the generic callables
    // (the CT parameters will not generate final params)
    int funCallNumArgs = funCall.arguments().size();
    if (referredFun.hasProperty(propResultParam))
        funCallNumArgs--;
    RC_ASSERT(funCallNumArgs <= callable.numParams());
}

TEST_CASE_METHOD(CallableFixture, "CallableFixture.GenericPackageCallable") {

    types_.init(*this, SampleTypes::addByteType);

    rc::prop("GenericPackageCallable can be called with proper arguments", [=]() {
        ParamsData paramsData;
        GenericPackageCallable callable = genGenericPackageCallable(paramsData);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        // Check that we can call the callable with the given args
        checkCanCall(callable, args);
        checkGenerateCall(callable);
    });

    rc::prop("GenericPackageCallable can be called with argument completion", [=]() {
        ParamsData paramsData;
        GenericPackageCallable callable = genGenericPackageCallable(paramsData);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        // If we reduce the arguments, we should be still able to call the callable
        // NOTE: to avoid comparing CT values, we should only call checkCanCall once per callable
        if (paramsData.idxStartInit_ < paramsData.numParams_) {
            int numArgs = *rc::gen::inRange(paramsData.idxStartInit_, paramsData.numParams_);

            checkCanCall(callable, NodeRange(args).shrinkTo(numArgs));
            checkGenerateCall(callable);
        }
    });

    rc::prop("test GenericPackageCallable with false if clause", [=]() {
        ParamsData paramsData;
        GenericPackageCallable callable = genGenericPackageCallable(paramsData, false);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        checkCannotCall(callable, args);
    });
}

TEST_CASE_METHOD(CallableFixture, "CallableFixture.GenericDatatypeCallable") {

    types_.init(*this, SampleTypes::addByteType);

    rc::prop("GenericDatatypeCallable can be called with proper arguments", [=]() {
        ParamsData paramsData;
        GenericDatatypeCallable callable = genGenericDatatypeCallable(paramsData);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        // Check that we can call the callable with the given args
        checkCanCall(callable, args);
        checkGenerateCall(callable);
    });

    rc::prop("GenericDatatypeCallable can be called with argument completion", [=]() {
        ParamsData paramsData;
        GenericDatatypeCallable callable = genGenericDatatypeCallable(paramsData);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        // If we reduce the arguments, we should be still able to call the callable
        // NOTE: to avoid comparing CT values, we should only call checkCanCall once per callable
        if (paramsData.idxStartInit_ < paramsData.numParams_) {
            int numArgs = *rc::gen::inRange(paramsData.idxStartInit_, paramsData.numParams_);

            checkCanCall(callable, NodeRange(args).shrinkTo(numArgs));
            checkGenerateCall(callable);
        }
    });

    rc::prop("test GenericDatatypeCallable with false if clause", [=]() {
        ParamsData paramsData;
        GenericDatatypeCallable callable = genGenericDatatypeCallable(paramsData, false);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        checkCannotCall(callable, args);
    });
}

TEST_CASE_METHOD(CallableFixture, "CallableFixture.FunctionCallable") {

    types_.init(*this, SampleTypes::addByteType);

    rc::prop("FunctionCallable can be called with proper arguments", [=]() {
        ParamsData paramsData;
        FunctionCallable callable = genFunctionCallable(paramsData);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        // Check that we can call the callable with the given args
        checkCanCall(callable, args);
        checkGenerateCall(callable);
    });

    rc::prop("FunctionCallable can be called with argument completion", [=]() {
        ParamsData paramsData;
        FunctionCallable callable = genFunctionCallable(paramsData);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        // If we reduce the arguments, we should be still able to call the callable
        // NOTE: to avoid comparing CT values, we should only call checkCanCall once per callable
        if (paramsData.idxStartInit_ < paramsData.numParams_) {
            int numArgs = *rc::gen::inRange(paramsData.idxStartInit_, paramsData.numParams_);

            checkCanCall(callable, NodeRange(args).shrinkTo(numArgs));
            checkGenerateCall(callable);
        }
    });
}

GenericFunctionCallable CallableFixture::getCallable(SprFunctionDecl funDecl) {
    // Semantic check
    funDecl.setContext(globalContext_);
    FeatherNodeFactory::instance().setContextForAuxNodes(globalContext_);
    RC_ASSERT(funDecl.semanticCheck());

    auto genFunDecl = NodeHandle(resultingDecl(funDecl)).kindCast<GenericFunction>();
    RC_ASSERT(genFunDecl);

    return GenericFunctionCallable(genFunDecl);
}

TypeWithStorage addRefs(TypeWithStorage t, int numRefs) {
    if (t.kind() == typeKindConcept) {
        return ConceptType::get(t.referredNode(), numRefs, t.mode());
    }
    else if (t.kind() == typeKindData) {
        return DataType::get(t.referredNode(), numRefs, t.mode());
    }
    else {
        for (int i=0; i<numRefs; i++)
            t = Feather::addRef(t);
        return t;
    }
}
void CallableFixture::testFunctionCallable(
        Range<TypeWithStorage> paramTypes, Range<TypeWithStorage> argTypes) {
    CHECK(paramTypes.size() == argTypes.size());
    Location loc = createLocation();

    // Build the params NodeList
    NodeList params;
    int i=0;
    for (auto paramType: paramTypes) {
        auto curParam = ParameterDecl::create(loc, concat("p", ++i), paramType);
        params = NodeList::append(params, curParam);
    }

    // Create the function decl and get the corresponding callable
    auto funDecl = genFunction(params);
    // printNode(funDecl);
    auto callable = getCallable(funDecl);

    // Generate arguments
    vector<NodeHandle> args;
    args.reserve(argTypes.size());
    for (auto t: argTypes)
        args.push_back(Feather::TypeNode::create(loc, t));
    semanticCheck(args, globalContext_);

    // Check that we can call the callable with the given args
    checkCanCall(callable, args);
    checkGenerateCall(callable);
}

TEST_CASE_METHOD(CallableFixture, "CallableFixture.GenericFunctionCallable") {

    types_.init(*this, SampleTypes::addByteType);

    // Add a 'typeOf' function
    auto typeOfFun = genTypeOfFunction(createLocation());
    typeOfFun.setContext(globalContext_);

    SECTION("Manual generic function callable tests") {
        testFunctionCallable(
                Range<TypeWithStorage>{ // param types
                        types_.concept1Type_,
                        types_.i8Type_,
                },
                Range<TypeWithStorage>{ // arg types
                        addRef(types_.barType_),
                        types_.i8Type_,
                });

        testFunctionCallable(
                Range<TypeWithStorage>{ // param types
                        types_.concept1Type_,
                        addRefs(types_.concept1Type_, 2),
                        types_.concept1Type_,
                },
                Range<TypeWithStorage>{ // arg types
                        addRef(types_.barType_),
                        MutableType::get(addRefs(types_.fooType_, 2)),
                        types_.fooType_,
                });
    }

    rc::prop("GenericFunctionCallable can be called with proper arguments", [=]() {
        ParamsData paramsData;
        GenericFunctionCallable callable = genGenericFunctionCallable(paramsData);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        // Check that we can call the callable with the given args
        checkCanCall(callable, args);
        checkGenerateCall(callable);
    });

    rc::prop("GenericFunctionCallable can be called with argument completion", [=]() {
        ParamsData paramsData;
        GenericFunctionCallable callable = genGenericFunctionCallable(paramsData);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        // If we reduce the arguments, we should be still able to call the callable
        // NOTE: to avoid comparing CT values, we should only call checkCanCall once per callable
        if (paramsData.idxStartInit_ < paramsData.numParams_) {
            int numArgs = *rc::gen::inRange(paramsData.idxStartInit_, paramsData.numParams_);

            checkCanCall(callable, NodeRange(args).shrinkTo(numArgs));
            checkGenerateCall(callable);
        }
    });

    rc::prop("test GenericFunctionCallable with false if clause", [=]() {
        ParamsData paramsData;
        GenericFunctionCallable callable = genGenericFunctionCallable(paramsData, false);

        // Generate some args that match the given types
        vector<NodeHandle> args = *arbArguments(paramsData, &types_);
        semanticCheck(args, globalContext_);

        checkCannotCall(callable, args);
    });
}
