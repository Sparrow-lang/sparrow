#include "StdInc.h"
#include "Common/RcBasic.hpp"
#include "Common/TypeFactory.hpp"
#include "Common/GeneralFixture.hpp"

#include "SparrowFrontend/Helpers/Convert.h"
#include "SparrowFrontend/Helpers/Overload.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Utils/Diagnostic.hpp"

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
            NodeRange decls, NodeRange args, OverloadReporting errReporting,
            StringRef funName) final {
        return nullptr;
    }

    bool selectConversionCtor(CompilationContext* context, Node* destClass, EvalMode destMode,
            TypeRef argType, Node* arg, Node** conv) final {
        if (argType->mode != destMode)
            return false;
        // If we know about the conversion, we return true
        TypeRef destType = Feather_getDataType(destClass, 0, destMode);
        for (const auto& p : implicitConversions_) {
            if (p.first == argType && p.second == destType)
                return true;
        }
        return false;
    }

    Node* selectCtToRtCtor(Node* ctArg) final { return nullptr; }

    vector<pair<TypeRef, TypeRef>> implicitConversions_;
};

struct ConvertFixture : GeneralFixture {
    ConvertFixture();
    ~ConvertFixture();

    ConversionType getConvType(TypeRef src, TypeRef dest, ConversionFlags flags = flagsDefault) {
        auto res = g_ConvertService->canConvertType(globalContext_, src, dest, flags);
        return res.conversionType();
    }

    TypeRef fooType_;
    TypeRef barType_; // fooType_ -> barType_
    TypeRef nullType_;
};

ConvertFixture::ConvertFixture() {
    // Mock the overload service
    delete g_OverloadService;
    auto mockOverloadService = new OverloadServiceMock;
    g_OverloadService = mockOverloadService;

    using TypeFactory::g_dataTypeDecls;

    // Create the basic types
    g_dataTypeDecls.push_back(createNativeDatatypeNode(fromCStr("i8"), globalContext_));
    g_dataTypeDecls.push_back(createNativeDatatypeNode(fromCStr("i16"), globalContext_));
    g_dataTypeDecls.push_back(createNativeDatatypeNode(fromCStr("i32"), globalContext_));
    // g_dataTypeDecls.push_back(createNativeDatatypeNode(fromCStr("i64"), globalContext_));
    // g_dataTypeDecls.push_back(createNativeDatatypeNode(fromCStr("float"), globalContext_));
    // g_dataTypeDecls.push_back(createNativeDatatypeNode(fromCStr("double"), globalContext_));
    Node* fooTypeDecl = createDatatypeNode(fromCStr("FooType"), globalContext_);
    Node* barTypeDecl = createDatatypeNode(fromCStr("BarType"), globalContext_);
    Node* nullTypeDecl = createDatatypeNode(fromCStr("NullType"), globalContext_);
    g_dataTypeDecls.push_back(fooTypeDecl);
    g_dataTypeDecls.push_back(barTypeDecl);
    g_dataTypeDecls.push_back(nullTypeDecl);

    // Create an implicit conversion from FooType to BarType
    fooType_ = Feather_getDataType(fooTypeDecl, 0, modeRt);
    barType_ = Feather_getDataType(barTypeDecl, 0, modeRt);
    mockOverloadService->implicitConversions_.emplace_back(make_pair(fooType_, barType_));

    // Ensure we set the Null type
    nullType_ = Feather_getDataType(nullTypeDecl, 0, modeRt);
    SprFrontend::StdDef::typeNull = nullType_;
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

    rc::prop("canConvertType doesn't crash", [=](TypeRef src, TypeRef dest) {
        (void)g_ConvertService->canConvertType(globalContext_, src, dest);
    });
}

TEST_CASE_METHOD(ConvertFixture, "Conversion rules are properly applied") {

    rc::prop("A type shall convert to itself (direct)", [=](TypeRef src) {
        auto res = g_ConvertService->canConvertType(globalContext_, src, src);
        RC_ASSERT(res.conversionType() == convDirect);
    });
    rc::prop("Unrelated types shall not have a conversion (none)", [=](TypeRef src, TypeRef dest) {
        RC_PRE(src != dest);
        RC_PRE(src->referredNode != dest->referredNode);
        RC_PRE(src->referredNode != nullType_->referredNode); // Null exception
        RC_PRE(src->referredNode != fooType_->referredNode ||
                dest->referredNode != barType_->referredNode); // Implicit conversion exception
        auto res = g_ConvertService->canConvertType(globalContext_, src, dest);
        RC_ASSERT(res.conversionType() == convNone);
    });
    rc::prop("Only void converts to void (none)", [=](TypeRef src) {
        RC_PRE(src->typeKind != Feather_getVoidTypeKind());
        TypeRef voidRt = Feather_getVoidType(modeRt);
        TypeRef voidCt = Feather_getVoidType(modeCt);
        RC_ASSERT(getConvType(src, voidRt) == convNone);
        RC_ASSERT(getConvType(src, voidCt) == convNone);
        RC_ASSERT(getConvType(voidCt, src) == convNone);
        RC_ASSERT(getConvType(voidRt, src) == convNone);
    });
    rc::prop("Convert from CT to RT shall work, in the absence of references (direct)", [=]() {
        TypeRef src = *TypeFactory::arbBasicStorageType(modeCt, 0, 1);
        TypeRef dest = Nest_changeTypeMode(src, modeRt);
        RC_LOG() << src << " -> " << dest << endl;
        RC_ASSERT(getConvType(src, dest) == convDirect);
    });

    rc::prop("if T and U are unrelated (basic storage), then lv(T)->U == none)", [=]() {
        TypeRef t = *TypeFactory::arbBasicStorageType();
        TypeRef u = *TypeFactory::arbBasicStorageType();
        RC_PRE(t->referredNode != u->referredNode);
        RC_PRE(t->referredNode != nullType_->referredNode);
        RC_PRE(t->referredNode != fooType_->referredNode ||
                u->referredNode != barType_->referredNode);
        RC_LOG() << t << " -> " << u;

        TypeRef lvT = Feather_getLValueType(t);
        RC_ASSERT(getConvType(lvT, u) == convNone);
    });

    rc::prop("if T->U (T=datatype) then lv(T)->U", [=]() {
        TypeRef t = *TypeFactory::arbDataType();
        TypeRef u = *TypeFactory::arbBasicStorageType();
        RC_PRE(t->referredNode == u->referredNode);
        TypeRef lvT = Feather_getLValueType(t);
        RC_PRE(lvT != u);
        RC_LOG() << lvT << " -> " << u << endl;

        ConversionType c1 = getConvType(t, u);
        RC_LOG() << "    " << t << " -> " << u << " = " << int(c1) << endl;

        if (c1)
            RC_ASSERT(getConvType(lvT, u) != convNone);
    });

    rc::prop("if @T->U (T=datatype) then lv(T)->U", [=]() {
        TypeRef t = *TypeFactory::arbDataType();
        TypeRef u = *TypeFactory::arbBasicStorageType();
        RC_PRE(t->referredNode == u->referredNode); // increase the chance of matching
        TypeRef lvT = Feather_getLValueType(t);
        RC_LOG() << lvT << " -> " << u << endl;

        TypeRef rt = Feather_addRef(t);
        ConversionType c1 = getConvType(rt, u);
        RC_LOG() << "    " << rt << " -> " << u << " = " << int(c1) << endl;

        if (c1) {
            RC_ASSERT(getConvType(lvT, u) != convNone);
        }
    });

    SECTION("LValue examples") {
        Node* decl = TypeFactory::g_dataTypeDecls[0];
        TypeRef t0 = Feather_getDataType(decl, 0, modeRt); // i8
        TypeRef t1 = Feather_getDataType(decl, 1, modeRt); // @i8
        TypeRef t2 = Feather_getDataType(decl, 2, modeRt); // @@i8
        TypeRef t0lv = Feather_getLValueType(t0);          // i8 lv
        TypeRef t1lv = Feather_getLValueType(t0);          // @i8 lv
        CHECK(getConvType(t0, t1) == convImplicit);
        CHECK(getConvType(t0lv, t0) == convDirect); // strange!
        CHECK(getConvType(t0lv, t1) == convImplicit);
        CHECK(getConvType(t1lv, t1) == convImplicit); // strange!
        CHECK(getConvType(t1lv, t2) == convNone);
        CHECK(getConvType(t1, t2) == convNone);
        CHECK(getConvType(t0, t2) == convNone);
    }

    rc::prop("Null type always converts to references (of storage types)", [=]() {
        TypeRef dest = *TypeFactory::arbBasicStorageType(modeUnspecified, 1, 4);
        TypeRef nullType = Nest_changeTypeMode(SprFrontend::StdDef::typeNull, dest->mode);
        RC_PRE(dest->referredNode != nullType->referredNode);
        RC_LOG() << nullType << " -> " << dest << endl;

        RC_ASSERT(getConvType(nullType, dest) == convImplicit);
    });

    rc::prop("if T -> U (don't add ref, don't cvt), T=datatype, then @T -> U (implicit)", [=]() {
        TypeRef src = *TypeFactory::arbDataType();
        TypeRef dest = *TypeFactory::arbType();
        TypeRef srcRef = Feather_addRef(src);
        RC_PRE(srcRef != dest);
        RC_LOG() << srcRef << " -> " << dest << endl;

        int flags = flagDontAddReference | flagDontCallConversionCtor;
        auto c1 = getConvType(src, dest, ConversionFlags(flags));
        RC_LOG() << "    " << src << " -> " << dest << " = " << int(c1) << endl;
        if (c1)
            RC_ASSERT(getConvType(srcRef, dest) == worstConv(c1, convImplicit));
    });

    rc::prop("if @T -> U, refs(T)==0, then T -> U (implicit)", [=]() {
        TypeRef src = *TypeFactory::arbDataType(modeUnspecified, 0, 1);
        TypeRef dest = *TypeFactory::arbType();
        TypeRef srcRef = Feather_addRef(src);
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
    // TODOs:
    // concept: #C@N -> Concept@N
    // concept: Concept1@N -> Concept2@N
}
