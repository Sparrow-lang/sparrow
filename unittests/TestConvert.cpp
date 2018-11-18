#include "StdInc.h"
#include "Common/RcBasic.hpp"
#include "Common/TypeFactory.hpp"
#include "Common/GeneralFixture.hpp"
#include "Common/BackendMock.hpp"

#include "SparrowFrontend/Helpers/Convert.h"
#include "SparrowFrontend/Helpers/Overload.h"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "SparrowFrontend/Utils/cppif/SparrowFrontendTypes.hpp"
#include "SparrowFrontend/Nodes/SprProperties.h"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/Diagnostic.hpp"

using namespace Feather;
using namespace SprFrontend;

namespace rc {

template <> struct Arbitrary<ConversionType> {
    static Gen<ConversionType> arbitrary() {
        return gen::element(convNone, convCustom, convConceptWithImplicit, convConcept,
                convImplicit, convDirect);
    }
};

} // namespace rc

struct OverloadServiceMock : IOverloadService {
    Node* selectOverload(CompilationContext* context, const Location& loc, EvalMode evalMode,
            Nest_NodeRange decls, Nest_NodeRange args, OverloadReporting errReporting,
            StringRef funName) final {
        return nullptr;
    }

    bool selectConversionCtor(CompilationContext* context, Node* destClass, EvalMode destMode,
            Type argType) final {
        if (argType.mode() != destMode)
            return false;
        // If we know about the conversion, we return true
        Type destType = DataType::get(destClass, 0, destMode);
        for (const auto& p : implicitConversions_) {
            if (p.first == argType && p.second == destType)
                return true;
        }
        return false;
    }

    Node* selectCtToRtCtor(Node* ctArg) final { return nullptr; }

    vector<pair<Type, Type>> implicitConversions_;
};

struct ConceptsServiceMock : IConceptsService {
    bool conceptIsFulfilled(Node* concept, Type type) final {
        for (auto p : conceptFulfillments_)
            if (p.first == concept && p.second.referredNode() == type.referredNode())
                return true;
        return false;
    }
    bool typeGeneratedFromGeneric(Node* genericDatatype, Type type) final { return false; }
    ConceptType baseConceptType(Node* concept) final {
        for (auto p : baseConcepts_)
            if (p.first == concept)
                return p.second;
        return nullptr;
    }

    vector<pair<Node*, Type>> conceptFulfillments_;
    vector<pair<Node*, ConceptType>> baseConcepts_; // concept -> base concept type
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct ConvertFixture : SparrowGeneralFixture {
    ConvertFixture();
    ~ConvertFixture();

    ConversionType getConvType(Type src, Type dest, ConversionFlags flags = flagsDefault) {
        auto res = g_ConvertService->checkConversion(globalContext_, src, dest, flags);
        return res.conversionType();
    }

    ConversionResult getConvResult(
            Type src, Type dest, ConversionFlags flags = flagsDefault) {
        return g_ConvertService->checkConversion(globalContext_, src, dest, flags);
    }

    //! Check category conversions
    void checkCatConversions(DataType src, DataType dest);


    DataType fooType_;
    DataType barType_; // fooType_ -> barType_
    DataType nullType_;
    ConceptType concept1Type_;
    ConceptType concept2Type_;
};

ConvertFixture::ConvertFixture() {
    // Mock the overload service
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto mockOverloadService = new OverloadServiceMock;
    g_OverloadService.reset(mockOverloadService);

    // Mock the concepts service
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto mockConceptsService = new ConceptsServiceMock;
    g_ConceptsService.reset(mockConceptsService);

    using TypeFactory::g_conceptDecls;
    using TypeFactory::g_dataTypeDecls;

    // Create the basic types
    g_dataTypeDecls.push_back(createNativeDatatypeNode(StringRef("i8"), globalContext_));
    g_dataTypeDecls.push_back(createNativeDatatypeNode(StringRef("i16"), globalContext_));
    g_dataTypeDecls.push_back(createNativeDatatypeNode(StringRef("i32"), globalContext_));
    Node* fooTypeDecl = createDatatypeNode(StringRef("FooType"), globalContext_);
    Node* barTypeDecl = createDatatypeNode(StringRef("BarType"), globalContext_);
    Node* nullTypeDecl = createDatatypeNode(StringRef("NullType"), globalContext_);
    g_dataTypeDecls.push_back(fooTypeDecl);
    g_dataTypeDecls.push_back(barTypeDecl);
    g_dataTypeDecls.push_back(nullTypeDecl);


    // Create an implicit conversion from FooType to BarType
    Nest::NodeHandle(barTypeDecl).setProperty(propConvert, 1);
    fooType_ = DataType::get(fooTypeDecl, 0, modeRt);
    barType_ = DataType::get(barTypeDecl, 0, modeRt);
    mockOverloadService->implicitConversions_.emplace_back(make_pair(fooType_, barType_));

    // Ensure we set the Null type
    nullType_ = DataType::get(nullTypeDecl, 0, modeRt);
    SprFrontend::StdDef::typeNull = nullType_;
    SprFrontend::StdDef::clsNull = nullTypeDecl;

    // Ensure we set the Type type -- but don't add it to our conversion types
    Node* typeDecl = createDatatypeNode(StringRef("Type"), globalContext_);
    SprFrontend::StdDef::typeType = DataType::get(typeDecl, 0, modeCt);
    REQUIRE(Nest_computeType(typeDecl) != nullptr);

    // Create concept types
    Node* concept1 = createSimpleConcept(StringRef("Concept1"), globalContext_);
    Node* concept2 = createSimpleConcept(StringRef("Concept2"), globalContext_);
    REQUIRE(Nest_computeType(concept1) != nullptr);
    REQUIRE(Nest_computeType(concept2) != nullptr);
    g_conceptDecls.push_back(concept1);
    g_conceptDecls.push_back(concept2);
    concept1Type_ = ConceptType::get(concept1);
    concept2Type_ = ConceptType::get(concept2);

    // Make concept -> type associations
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept1, fooType_));
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept1, barType_));
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept2, fooType_));
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept2, barType_));

    // Concept base
    mockConceptsService->baseConcepts_.emplace_back(make_pair(concept2, concept1Type_));
}

ConvertFixture::~ConvertFixture() {}

TEST_CASE("User shall be able to combine two ConversionType values") {
    SECTION("combine function") {
        rc::prop("combining with none yields none", [](ConversionType ct) {
            auto res = combine(ct, convNone);
            RC_ASSERT(res == convNone);

            auto res2 = combine(convNone, convNone);
            RC_ASSERT(res2 == convNone);
        });
        rc::prop("combining cannot yield a better conversion",
                [](ConversionType ct1, ConversionType ct2) {
                    auto res = combine(ct1, ct2);
                    auto worst = worstConv(ct1, ct2);
                    RC_ASSERT(int(res) <= int(ct1));
                    RC_ASSERT(int(res) <= int(ct2));
                    RC_ASSERT(int(res) <= int(worst));
                });
        rc::prop("combining same type yields the same type", [](ConversionType ct) {
            auto res = combine(ct, ct);
            RC_ASSERT(res == ct);
        });
        rc::prop("except concept+implicit, combining shall not produce a different type",
                [](ConversionType ct1, ConversionType ct2) {
                    if ((ct1 != convConcept || ct2 != convImplicit) &&
                            (ct1 != convImplicit || ct2 != convConcept)) {
                        auto res = combine(ct1, ct2);
                        RC_ASSERT(res == ct1 || res == ct2);
                    }
                });

        // Direct checks
        REQUIRE(combine(convConcept, convImplicit) == convConceptWithImplicit);
        REQUIRE(combine(convImplicit, convConcept) == convConceptWithImplicit);
    }

    SECTION("worstConv functions") {
        rc::prop("worstConv always returns one of its inputs",
                [](ConversionType ct1, ConversionType ct2) {
                    auto res = worstConv(ct1, ct2);
                    RC_ASSERT(res == ct1 || res == ct2);
                });
        rc::prop("worstConv always returns the smallest input",
                [](ConversionType ct1, ConversionType ct2) {
                    auto res = worstConv(ct1, ct2);
                    RC_ASSERT(int(res) <= int(ct1));
                    RC_ASSERT(int(res) <= int(ct2));
                });
    }

    SECTION("bestConv functions") {
        rc::prop("bestConv always returns one of its inputs",
                [](ConversionType ct1, ConversionType ct2) {
                    auto res = bestConv(ct1, ct2);
                    RC_ASSERT(res == ct1 || res == ct2);
                });
        rc::prop("bestConv always returns the biggest input",
                [](ConversionType ct1, ConversionType ct2) {
                    auto res = bestConv(ct1, ct2);
                    RC_ASSERT(int(res) >= int(ct1));
                    RC_ASSERT(int(res) >= int(ct2));
                });
    }
}

TEST_CASE_METHOD(ConvertFixture, "User shall be able to check conversion between any two types") {

    rc::prop("checkConversion doesn't crash", [=](Type src, Type dest) {
        (void)g_ConvertService->checkConversion(globalContext_, src, dest);
    });
}

TEST_CASE_METHOD(ConvertFixture, "Conversion rules are properly applied") {

    rc::prop("A type shall convert to itself (direct)",
            [=](Type src) { RC_ASSERT(getConvType(src, src) == convDirect); });
    rc::prop("Unrelated types shall not have a conversion (none)", [=](Type src, Type dest) {
        RC_PRE(src != dest);
        RC_PRE(src.referredNode() != dest.referredNode());
        RC_PRE(src.referredNode() != nullType_.referredNode()); // Null exception
        RC_PRE(src.referredNode() != fooType_.referredNode() ||
                dest.referredNode() != barType_.referredNode()); // Implicit conversion exception
        RC_PRE(dest.kind() != SprFrontend::typeKindConcept);
        auto res = g_ConvertService->checkConversion(globalContext_, src, dest);
        RC_ASSERT(res.conversionType() == convNone);
    });
    rc::prop("Only void converts to void (none)", [=](Type src) {
        RC_PRE(src.kind() != Feather_getVoidTypeKind());
        Type voidRt = Feather_getVoidType(modeRt);
        Type voidCt = Feather_getVoidType(modeCt);
        RC_ASSERT(getConvType(src, voidRt) == convNone);
        RC_ASSERT(getConvType(src, voidCt) == convNone);
        RC_ASSERT(getConvType(voidCt, src) == convNone);
        RC_ASSERT(getConvType(voidRt, src) == convNone);
    });
    rc::prop("Convert from CT to RT shall work, in the absence of references (direct)", [=]() {
        Type src = *TypeFactory::arbBasicStorageType(modeCt, 0, 1);
        Type dest = Nest_changeTypeMode(src, modeRt);
        RC_LOG() << src << " -> " << dest << endl;
        RC_ASSERT(getConvType(src, dest) == convDirect);
    });

    rc::prop("if T and U are unrelated (basic storage), then mut(T)->U == none)", [=]() {
        auto t = *TypeFactory::arbDataType();
        auto u = *TypeFactory::arbBasicStorageType();
        RC_PRE(t.referredNode() != u.referredNode());
        RC_PRE(t.referredNode() != nullType_.referredNode());
        RC_PRE(t.referredNode() != fooType_.referredNode() ||
                u.referredNode() != barType_.referredNode());
        RC_LOG() << t << " -> " << u;

        Type mutT = MutableType::get(t);
        RC_ASSERT(getConvType(mutT, u) == convNone);
    });

    rc::prop("if T->U (T=datatype) then mut(T)->U", [=]() {
        auto t = *TypeFactory::arbDataType();
        auto u = *TypeFactory::arbBasicStorageType();
        RC_PRE(t.referredNode() == u.referredNode());
        Type mutT = MutableType::get(t);
        RC_PRE(mutT != u);
        RC_LOG() << mutT << " -> " << u << endl;

        ConversionType c1 = getConvType(t, u);
        RC_LOG() << "    " << t << " -> " << u << " = " << int(c1) << endl;

        if (c1)
            RC_ASSERT(getConvType(mutT, u) != convNone);
    });

    rc::prop("if @T->U (T=datatype) then lv(T)->U", [=]() {
        auto t = *TypeFactory::arbDataType();
        auto u = *TypeFactory::arbBasicStorageType();
        RC_PRE(t.referredNode() == u.referredNode()); // increase the chance of matching
        Type mutT = MutableType::get(t);
        RC_LOG() << mutT << " -> " << u << endl;

        Type rt = addRef(DataType(t));
        ConversionType c1 = getConvType(rt, u);
        RC_LOG() << "    " << rt << " -> " << u << " = " << int(c1) << endl;

        if (c1) {
            RC_ASSERT(getConvType(mutT, u) != convNone);
        }
    });

    SECTION("MutableType examples") {
        Node* decl = TypeFactory::g_dataTypeDecls[0];
        auto t0 = DataType::get(decl, 0, modeRt);       // i8
        auto t1 = DataType::get(decl, 1, modeRt);       // @i8
        auto t2 = DataType::get(decl, 2, modeRt);       // @@i8
        auto t0mut = MutableType::get(t0);              // i8 mut
        auto t1mut = MutableType::get(t1);              // @i8 mut
        CHECK(getConvType(t0, t1) == convImplicit);
        CHECK(getConvType(t0mut, t0) == convDirect);
        CHECK(getConvType(t0mut, t1) == convImplicit);
        CHECK(getConvType(t1mut, t1) == convDirect);
        CHECK(getConvType(t1mut, t2) == convImplicit);
        CHECK(getConvType(t1, t2) == convNone);
        CHECK(getConvType(t0, t2) == convNone);
    }

    rc::prop("Null type always converts to references (of storage types)", [=]() {
        auto dest = *TypeFactory::arbBasicStorageType(modeUnspecified, 1, 4);
        auto nullType = nullType_.changeMode(dest.mode());
        RC_PRE(dest.referredNode() != nullType.referredNode());
        RC_LOG() << nullType << " -> " << dest << endl;

        RC_ASSERT(getConvType(nullType, dest) == convImplicit);
    });

    rc::prop("if T -> U (don't add ref, don't cvt), T=datatype, then @T -> U (implicit)", [=]() {
        auto src = *TypeFactory::arbDataType();
        auto dest = *TypeFactory::arbType();
        auto srcRef = addRef(DataType(src));
        RC_PRE(srcRef != dest);
        RC_LOG() << srcRef << " -> " << dest << endl;

        int flags = flagDontAddReference | flagDontCallConversionCtor;
        auto c1 = getConvType(src, dest, ConversionFlags(flags));
        RC_LOG() << "    " << src << " -> " << dest << " = " << int(c1) << endl;
        if (c1)
            RC_ASSERT(getConvType(srcRef, dest) != convNone);
    });

    rc::prop("if @T -> U, refs(T)==0, then T -> U (implicit)", [=]() {
        auto src = *TypeFactory::arbDataType(modeUnspecified, 0, 1);
        auto dest = *TypeFactory::arbType();
        auto srcRef = addRef(DataType(src));
        RC_PRE(srcRef != dest);
        RC_LOG() << src << " -> " << dest << endl;

        auto c1 = getConvType(srcRef, dest);
        RC_LOG() << "    " << srcRef << " -> " << dest << " = " << int(c1) << endl;
        if (c1)
            RC_ASSERT(getConvType(src, dest) != convNone);
    });

    SECTION("Conversion ctor example") {
        CHECK(getConvType(fooType_, barType_) == convCustom);
        // Other derived types arrive at conversion through other (composed) rules
    }

    rc::prop("if a datatype fulfills a concept, derived types will also fulfill it", [=]() {
        TypeWithStorage src = *rc::gen::element(fooType_, barType_);
        int numRefs = *rc::gen::inRange(0, 4);
        bool useMut = *rc::gen::element(0, 1) != 0;
        for (int i = 0; i < numRefs; i++)
            src = addRef(src);
        if (useMut)
            src = MutableType::get(src);
        auto dest = *TypeFactory::arbConceptType(src.mode(), 0, 1);

        RC_LOG() << src << " -> " << dest << endl;
        RC_ASSERT(getConvType(src, dest) != convNone);
    });

    SECTION("Concept base conversion") {
        CHECK(getConvType(concept1Type_, concept2Type_) == convNone);
        CHECK(getConvType(concept2Type_, concept1Type_) == convDirect);
    }
}

TEST_CASE_METHOD(ConvertFixture, "Conversion return types follow rules") {

    rc::prop("Checking rules for conversion types", [=](Type src, Type dest) {
        auto cvt = getConvType(src, dest);
        if (cvt != convNone) {
            RC_LOG() << src << " -> " << dest << endl;

            ConversionType expectedConv = convDirect;

            // If we have data-like types, and they point to different decls, this must be a
            // custom conversion
            // Exception: src == Null
            if (isDataLikeType(src) && isDataLikeType(dest) &&
                    src.referredNode() != dest.referredNode() &&
                    src.referredNode() != nullType_.referredNode())
                expectedConv = convCustom;

            // Check for "concept" conversions
            // That is the destination is a concept, without the source being a concept.
            if (src.kind() != SprFrontend::typeKindConcept &&
                    dest.kind() == SprFrontend::typeKindConcept)
                expectedConv = convConcept;

            unsigned srcBaseReferences = src.numReferences();
            if (Feather::isCategoryType(src))
                srcBaseReferences--;
            unsigned destBaseReferences = dest.numReferences();
            if (Feather::isCategoryType(dest))
                destBaseReferences--;

            // We don't have a good model for Null -> Null conversions
            // TODO (types)
            RC_PRE(src.referredNode() != nullType_.referredNode() ||
                    dest.referredNode() != nullType_.referredNode());

            // We don't have a good model for const/temp -> Concept conversions
            // TODO (types)
            if ((src.kind() == typeKindConst || src.kind() == typeKindTemp) &&
                    dest.kind() == typeKindConcept)
                RC_PRE(false);

            // Check for implicit conversions.
            // That is, whenever we do some conversions based on references
            bool isImplicit = false;
            if (src.referredNode() == nullType_.referredNode() &&
                    dest.referredNode() != nullType_.referredNode() && dest.numReferences() > 0)
                isImplicit = true;
            if (srcBaseReferences != destBaseReferences)
                isImplicit = true;
            if (isImplicit) {
                if (expectedConv == convConcept)
                    expectedConv = convConceptWithImplicit;
                else if (expectedConv == convDirect)
                    expectedConv = convImplicit;
            }

            RC_ASSERT(cvt == expectedConv);
        }
    });
}

//! For a given conversion, check that actions match conversion type
void checkActionsAgainstType(const ConversionResult& cvt, Type destType) {
    RC_LOG() << cvt << endl;

    // If we don't have a conversion, expect no actions
    if (!cvt) {
        RC_ASSERT(cvt.convertActions().empty());
        return;
    }

    bool isConcept = destType.kind() == SprFrontend::typeKindConcept;

    // Compute the expected conversion
    ConversionType minConv = convDirect;
    int i = 0;
    for (auto act : cvt.convertActions()) {
        switch (act.first) {
        case ActionType::none:
        case ActionType::modeCast:
            break;
        case ActionType::dereference:
            minConv = worstConv(minConv, convImplicit);
            break;
        case ActionType::bitcast:
            minConv = worstConv(minConv, convImplicit);
            break;
        case ActionType::makeNull:
            minConv = worstConv(minConv, convImplicit);
            break;
        case ActionType::addRef:
            minConv = worstConv(minConv, convImplicit);
            break;
        case ActionType::customCvt:
            minConv = worstConv(minConv, convCustom);
            break;
        }
    }

    if (isConcept) {
        if (minConv == convDirect)
            minConv = convConcept;
        else if (minConv == convImplicit)
            minConv = convConceptWithImplicit;
    }
    RC_ASSERT(cvt.conversionType() >= minConv);
}

//! Check different properties of action types
void checkActionTypes(const ConversionResult& cvt, Type srcType, Type destType) {
    if (!cvt)
        return;

    RC_LOG() << srcType << " -> " << destType << " : " << cvt << endl;

    // Obtain only the action types from the conversion
    static vector<ActionType> actions;
    actions.clear();
    actions.reserve(cvt.convertActions().size());
    transform(cvt.convertActions().begin(), cvt.convertActions().end(), back_inserter(actions),
            [](ConvAction ca) { return ca.first; });

    auto first = actions.begin();
    auto last = actions.end();

    // Check that we don't have both dereference and addRef actions
    // Exception: X lv/ct -> @X
    if (srcType.mode() != modeCt || destType.mode() != modeRt) {
        bool hasDeref = last != find(first, last, ActionType::dereference);
        bool hasAddRef = last != find(first, last, ActionType::addRef);
        RC_ASSERT(!hasDeref || !hasAddRef);
    }

    // Check that we have maximum 1 bitcast action
    RC_ASSERT(count(first, last, ActionType::bitcast) <= 1);

    // Check that we have maximum 1 custom covert action
    RC_ASSERT(count(first, last, ActionType::customCvt) <= 1);

    // Check that we have maximum 1 add-ref action
    RC_ASSERT(count(first, last, ActionType::addRef) <= 1);

    // Make-null action is the last one
    auto idx = find(first, last, ActionType::makeNull) - first;
    RC_ASSERT(idx + 1 >= actions.size());

    // Check that, at each step applying the conversion makes sense
    Type t = srcType;
    for (auto act : cvt.convertActions()) {
        Type newT = act.second;
        RC_LOG() << "  " << t << " -> " << newT << " : " << act.first << endl;

        switch (act.first) {
        case ActionType::none:
            RC_ASSERT(false);
            break;
        case ActionType::modeCast:
            RC_ASSERT(t.mode() == modeCt);
            RC_ASSERT(newT.mode() == modeRt);
            RC_ASSERT(t.kind() == newT.kind());
            RC_ASSERT(t.numReferences() == 0);
            RC_ASSERT(newT.numReferences() == 0);
            RC_ASSERT(t.referredNode() == newT.referredNode());
            t = newT;
            break;
        case ActionType::dereference:
            RC_ASSERT(t.mode() == newT.mode());
            RC_ASSERT(t.numReferences() == 1+newT.numReferences());
            RC_ASSERT(t.referredNode() == newT.referredNode());
            t = newT;
            break;
        case ActionType::bitcast:
            RC_ASSERT(t.mode() == newT.mode());
            RC_ASSERT(t.numReferences() > 0);
            RC_ASSERT(newT.numReferences() > 0);
            if ( t.kind() == typeKindData && newT.kind() != typeKindData)
                RC_ASSERT(t.numReferences() == newT.numReferences() ||
                          t.numReferences() + 1 == newT.numReferences());
            else
                RC_ASSERT(t.numReferences() == newT.numReferences());
            RC_ASSERT(t.referredNode() == newT.referredNode());
            t = newT;
            break;
        case ActionType::makeNull:
            RC_ASSERT(newT.numReferences() > 0);
            t = newT;
            break;
        case ActionType::addRef:
            RC_ASSERT(t.mode() == newT.mode());
            RC_ASSERT(t.numReferences()+1 == newT.numReferences());
            RC_ASSERT(t.referredNode() == newT.referredNode());
            t = newT;
            break;
        case ActionType::customCvt:
            RC_ASSERT(t.mode() == newT.mode());
            RC_ASSERT(t.referredNode() != newT.referredNode());
            t = newT;
            break;
        }
    }
    // At the end, the resulting type must be equal to the destination type
    // (except when the destination is a concept)
    if (destType.kind() != typeKindConcept) {
        RC_LOG() << "  (final) " << t << " == " << destType << " ?" << endl;

        RC_ASSERT(Nest::sameTypeIgnoreMode(t, destType));
    }
}

TEST_CASE_METHOD(ConvertFixture, "Convert actions applied follow rules") {
    rc::prop("convert actions match conversion type (related data types)", [=]() {
        Type src = *TypeFactory::arbBasicStorageType();
        Type dest = *TypeFactory::arbBasicStorageType();
        RC_PRE(src.referredNode() == dest.referredNode()); // increase the chance of matching
        RC_LOG() << src << " -> " << dest << endl;
        checkActionsAgainstType(getConvResult(src, dest), dest);
    });
    rc::prop("convert actions match conversion type (basic storage types)", [=]() {
        Type src = *TypeFactory::arbBasicStorageType();
        Type dest = *TypeFactory::arbBasicStorageType();
        RC_LOG() << src << " -> " << dest << endl;
        checkActionsAgainstType(getConvResult(src, dest), dest);
    });
    rc::prop("convert actions match conversion type (all types)", [=]() {
        Type src = *TypeFactory::arbType();
        Type dest = *TypeFactory::arbType();
        RC_LOG() << src << " -> " << dest << endl;
        checkActionsAgainstType(getConvResult(src, dest), dest);
    });

    rc::prop("no conversion implies no actions", [=]() {
        Type src = *TypeFactory::arbType();
        Type dest = *TypeFactory::arbType();
        RC_LOG() << src << " -> " << dest << endl;
        auto cvt = getConvResult(src, dest);
        if (!cvt)
            RC_ASSERT(cvt.convertActions().empty());
    });

    rc::prop("different properties of action types hold (related data types)", [=]() {
        Type src = *TypeFactory::arbBasicStorageType();
        Type dest = *TypeFactory::arbBasicStorageType();
        RC_PRE(src.referredNode() == dest.referredNode()); // increase the chance of matching
        checkActionTypes(getConvResult(src, dest), src, dest);
    });
    rc::prop("different properties of action types hold (basic storage types)", [=]() {
        Type src = *TypeFactory::arbBasicStorageType();
        Type dest = *TypeFactory::arbBasicStorageType();
        checkActionTypes(getConvResult(src, dest), src, dest);
    });
    rc::prop("different properties of action types hold (all types)", [=]() {
        Type src = *TypeFactory::arbType();
        Type dest = *TypeFactory::arbType();
        checkActionTypes(getConvResult(src, dest), src, dest);
    });
}

void ConvertFixture::checkCatConversions(DataType src, DataType dest) {
    auto baseConv = getConvType(src, dest);
    if (baseConv == convNone || baseConv == convCustom)
        return;

    RC_LOG() << src << " -> " << dest << endl;

    ConstType constSrc = ConstType::get(src);
    MutableType mutSrc = MutableType::get(src);
    TempType tmpSrc = TempType::get(src);
    ConstType constDest = ConstType::get(dest);
    MutableType mutDest = MutableType::get(dest);
    TempType tmpDest = TempType::get(dest);

    // Assume just RT cases; in CT, we may have less results
    // i.e., T/ct tmp -> T tmp needs T -> T tmp, which is invalid
    RC_PRE(src.mode() == modeRt);

    // Direct: T -> const(U), if T->U
    RC_ASSERT(getConvType(src, constDest) == baseConv);
    // Direct: const(T)->const(U), if T->U
    RC_ASSERT(getConvType(constSrc, constDest) == baseConv);
    // Direct: const(T)->plain(U), if T->U
    RC_ASSERT(getConvType(constSrc, dest) == baseConv);

    // Direct: mut(T)->mut(U), if T->U
    // RC_ASSERT(getConvType(mutSrc, mutDest) == baseConv);
    // Direct: mut(T)->const(U), if T->U
    RC_ASSERT(getConvType(mutSrc, constDest) == baseConv);
    // Direct: mut(T)->plain(U), if T->U
    RC_ASSERT(getConvType(mutSrc, dest) == baseConv);


    // Direct: tmp(T)->tmp(U), if T->U
    RC_ASSERT(getConvType(tmpSrc, tmpDest) == baseConv);
    // Direct: tmp(T)->const(U), if T->U
    RC_ASSERT(getConvType(tmpSrc, constDest) == baseConv);
    // Direct: tmp(T)->mut(U), if T->U
    RC_ASSERT(getConvType(tmpSrc, mutDest) == baseConv);
    // Direct: tmp(T)->plain(U), if T->U
    RC_ASSERT(getConvType(tmpSrc, dest) == baseConv);
}

TEST_CASE_METHOD(ConvertFixture, "Category conversions") {
    rc::prop("Category conversions work as expected (related data types)", [=]() {
        DataType src = *TypeFactory::arbDataType();
        DataType dest = *TypeFactory::arbDataType();
        RC_PRE(src.referredNode() == dest.referredNode()); // increase the chance of matching
        checkCatConversions(src, dest);
    });
    rc::prop("Category conversions work as expected (basic storage types)", [=]() {
        DataType src = *TypeFactory::arbDataType();
        DataType dest = *TypeFactory::arbDataType();
        checkCatConversions(src, dest);
    });
}
