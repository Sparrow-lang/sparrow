#include "StdInc.h"
#include "TypeFactory.hpp"
#include "RcBasic.hpp"

#include "Nest/Api/Type.h"
#include "SparrowFrontend/SparrowFrontendTypes.h"

namespace TypeFactory {

using namespace rc;
using namespace Feather;

vector<NodeHandle> g_dataTypeDecls{};
vector<NodeHandle> g_conceptDecls{};

Gen<VoidType> arbVoidType() { return gen::apply(&VoidType::get, gen::arbitrary<EvalMode>()); }

Gen<DataType> arbDataType(EvalMode mode, int minRef, int maxRef) {
    const int numT = g_dataTypeDecls.size();
    REQUIRE(numT > 0);
    auto modeGen = mode == modeUnspecified ? gen::arbitrary<EvalMode>() : gen::just(mode);
    return gen::apply(
            [=](int idx, int numReferences, EvalMode mode) -> DataType {
                REQUIRE(idx < g_dataTypeDecls.size());
                return DataType::get(g_dataTypeDecls[idx], numReferences, mode);
            },
            gen::inRange(0, numT), gen::inRange(minRef, maxRef), modeGen);
}

Gen<LValueType> arbLValueType(EvalMode mode, int minRef, int maxRef) {
    if (minRef > 0)
        minRef--;
    if (maxRef > 0)
        maxRef--;
    return gen::apply(&LValueType::get, arbDataType(mode, minRef, maxRef));
}

Gen<ArrayType> arbArrayType(EvalMode mode) {
    return gen::apply(
            [=](TypeRef base, unsigned count) -> ArrayType { return ArrayType::get(base, count); },
            arbDataType(mode), gen::inRange(1, 100));
}

Gen<FunctionType> arbFunctionType(EvalMode mode) {
    return gen::exec([=]() -> FunctionType {
        EvalMode m = mode == modeUnspecified ? *gen::arbitrary<EvalMode>() : mode;
        int numTypes = *gen::inRange(1, 5);
        vector<TypeRef> types;
        types.resize(numTypes);
        for (auto& t : types) {
            t = *arbDataType(m);
        }
        return FunctionType::get(&types[0], numTypes, m);
    });
}

Gen<TypeBase> arbConceptType(EvalMode mode, int minRef, int maxRef) {
    const int numT = g_conceptDecls.size();
    REQUIRE(numT > 0);
    auto modeGen = mode == modeUnspecified ? gen::arbitrary<EvalMode>() : gen::just(mode);
    return gen::apply(
            [=](int idx, int numReferences, EvalMode mode) -> TypeBase {
                REQUIRE(idx < g_conceptDecls.size());
                return SprFrontend::getConceptType(g_conceptDecls[idx], numReferences, mode);
            },
            gen::inRange(0, numT), gen::inRange(minRef, maxRef), modeGen);
}

Gen<TypeWithStorage> arbTypeWithStorage(EvalMode mode, int minRef, int maxRef) {
    return gen::exec([=]() -> TypeWithStorage {
        int idx = *gen::inRange(0, 4);
        if (minRef == 0 && idx == 1)
            idx = 0;
        switch (idx) {
        case 1:
            return *arbLValueType(mode, minRef, maxRef);
        case 2:
            return *arbArrayType(mode);
        case 3:
            return *arbFunctionType(mode);
        case 0:
        default:
            return *arbDataType(mode, minRef, maxRef);
        }
    });
}

Gen<TypeWithStorage> arbBasicStorageType(EvalMode mode, int minRef, int maxRef) {
    return gen::exec([=]() -> TypeWithStorage {
        if (minRef == 0 || *gen::inRange(0, 1) == 0)
            return *arbDataType(mode, minRef, maxRef);
        else
            return *arbLValueType(mode, minRef, maxRef);
    });
}

Gen<TypeBase> arbType() {
    return gen::exec([=]() -> TypeBase {
        switch (*gen::inRange(0, 6)) {
        case 1:
            return *arbLValueType();
        case 2:
            return *arbArrayType();
        case 3:
            return *arbFunctionType();
        case 4:
            return *arbVoidType();
        case 5:
            return *arbConceptType();
        case 0:
        default:
            return *arbDataType();
        }
    });
}

Gen<TypeRef> arbTypeRef() {
    return gen::map(TypeFactory::arbType(), [](Feather::TypeBase t) -> TypeRef { return t; });
}

} // namespace TypeFactory
