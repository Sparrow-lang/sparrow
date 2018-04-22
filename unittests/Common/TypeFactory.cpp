#include "StdInc.h"
#include "TypeFactory.hpp"
#include "RcBasic.hpp"

#include "Nest/Api/Type.h"
#include "SparrowFrontend/SparrowFrontendTypes.h"

namespace TypeFactory {

using namespace rc;

vector<Node*> g_dataTypeDecls{};
vector<Node*> g_conceptDecls{};

Gen<TypeRef> arbVoidType() { return gen::apply(&Feather_getVoidType, gen::arbitrary<EvalMode>()); }

Gen<TypeRef> arbDataType(EvalMode mode, int minRef, int maxRef) {
    const int numT = g_dataTypeDecls.size();
    REQUIRE(numT > 0);
    auto modeGen = mode == modeUnspecified ? gen::arbitrary<EvalMode>() : gen::just(mode);
    return gen::apply(
            [=](int idx, int numReferences, EvalMode mode) -> TypeRef {
                REQUIRE(idx < g_dataTypeDecls.size());
                return Feather_getDataType(g_dataTypeDecls[idx], numReferences, mode);
            },
            gen::inRange(0, numT), gen::inRange(minRef, maxRef), modeGen);
}

Gen<TypeRef> arbLValueType(EvalMode mode, int minRef, int maxRef) {
    if (minRef > 0)
        minRef--;
    if (maxRef > 0)
        maxRef--;
    return gen::apply(&Feather_getLValueType, arbDataType(mode, minRef, maxRef));
}

Gen<TypeRef> arbArrayType(EvalMode mode) {
    return gen::apply(
            [=](TypeRef base, unsigned count) -> TypeRef {
                return Feather_getArrayType(base, count);
            },
            arbDataType(mode), gen::inRange(1, 100));
}

Gen<TypeRef> arbFunctionType(EvalMode mode) {
    return gen::exec([=]() -> TypeRef {
        EvalMode m = mode == modeUnspecified ? *gen::arbitrary<EvalMode>() : mode;
        int numTypes = *gen::inRange(1, 5);
        vector<TypeRef> types;
        types.resize(numTypes);
        for (auto& t : types) {
            t = *arbDataType(m);
        }
        return Feather_getFunctionType(&types[0], numTypes, m);
    });
}

Gen<TypeRef> arbConceptType(EvalMode mode, int minRef, int maxRef) {
    const int numT = g_conceptDecls.size();
    REQUIRE(numT > 0);
    auto modeGen = mode == modeUnspecified ? gen::arbitrary<EvalMode>() : gen::just(mode);
    return gen::apply(
            [=](int idx, int numReferences, EvalMode mode) -> TypeRef {
                REQUIRE(idx < g_conceptDecls.size());
                return SprFrontend::getConceptType(g_conceptDecls[idx], numReferences, mode);
            },
            gen::inRange(0, numT), gen::inRange(minRef, maxRef), modeGen);
}

Gen<TypeRef> arbTypeWithStorage(EvalMode mode, int minRef, int maxRef) {
    if (minRef > 0)
        return gen::oneOf(arbDataType(mode, minRef, maxRef), arbLValueType(mode, minRef, maxRef),
                arbArrayType(), arbFunctionType());
    else
        return gen::oneOf(arbDataType(mode, minRef, maxRef), arbArrayType(), arbFunctionType());
}

Gen<TypeRef> arbBasicStorageType(EvalMode mode, int minRef, int maxRef) {
    if (minRef > 0)
        return gen::oneOf(arbDataType(mode, minRef, maxRef), arbLValueType(mode, minRef, maxRef));
    else
        return gen::oneOf(arbDataType(mode, minRef, maxRef));
}

Gen<TypeRef> arbType() {
    return gen::oneOf(arbDataType(), arbVoidType(), arbLValueType(), arbArrayType(),
            arbFunctionType(), arbConceptType());
}

} // namespace TypeFactory
