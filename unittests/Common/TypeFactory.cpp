#include "StdInc.h"
#include "TypeFactory.hpp"
#include "RcBasic.hpp"

#include "Nest/Api/Type.h"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"
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

Gen<ConstType> arbConstType(EvalMode mode, int minRef, int maxRef) {
    if (minRef > 0)
        minRef--;
    if (maxRef > 0)
        maxRef--;
    return gen::apply(&ConstType::get, arbDataType(mode, minRef, maxRef));
}

Gen<MutableType> arbMutableType(EvalMode mode, int minRef, int maxRef) {
    if (minRef > 0)
        minRef--;
    if (maxRef > 0)
        maxRef--;
    return gen::apply(&MutableType::get, arbDataType(mode, minRef, maxRef));
}

Gen<TempType> arbTempType(EvalMode mode, int minRef, int maxRef) {
    if (minRef > 0)
        minRef--;
    if (maxRef > 0)
        maxRef--;
    return gen::apply(&TempType::get, arbDataType(mode, minRef, maxRef));
}

Gen<ArrayType> arbArrayType(EvalMode mode) {
    return gen::apply(
            [=](TypeRef base, unsigned count) -> ArrayType { return ArrayType::get(base, count); },
            arbDataType(mode), gen::inRange(1, 100));
}

Gen<FunctionType> arbFunctionType(EvalMode mode, Nest::TypeWithStorage resType) {
    return gen::exec([=]() -> FunctionType {
        EvalMode m = mode == modeUnspecified ? *gen::arbitrary<EvalMode>() : mode;
        int numTypes = *gen::inRange(1, 5);
        vector<TypeRef> types;
        types.resize(numTypes);
        for (int i = 0; i < numTypes; i++) {
            auto t = i == 0 && resType ? resType : *arbDataType(m);
            types[i] = t;
        }
        return FunctionType::get(&types[0], numTypes, m);
    });
}

Gen<Type> arbConceptType(EvalMode mode, int minRef, int maxRef) {
    const int numT = g_conceptDecls.size();
    REQUIRE(numT > 0);
    auto modeGen = mode == modeUnspecified ? gen::arbitrary<EvalMode>() : gen::just(mode);
    return gen::apply(
            [=](int idx, int numReferences, EvalMode mode) -> Type {
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
            return *arbMutableType(mode, minRef, maxRef);
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
    int weightDataType = 5;
    int weightConstType = minRef == 0 ? 0 : 3;
    int weightMutableType = minRef == 0 ? 0 : 3;
    int weightTempType = 1;
    return gen::weightedOneOf<TypeWithStorage>({
            {weightDataType, gen::cast<TypeWithStorage>(arbDataType(mode, minRef, maxRef))},
            {weightConstType, gen::cast<TypeWithStorage>(arbConstType(mode, minRef, maxRef))},
            {weightMutableType, gen::cast<TypeWithStorage>(arbMutableType(mode, minRef, maxRef))},
            {weightTempType, gen::cast<TypeWithStorage>(arbTempType(mode, minRef, maxRef))},
    });
}

Gen<Type> arbType() {
    return gen::weightedOneOf<Type>({
            {5, gen::cast<Type>(arbDataType())},
            {3, gen::cast<Type>(arbConstType())},
            {3, gen::cast<Type>(arbMutableType())},
            {2, gen::cast<Type>(arbTempType())},
            {1, gen::cast<Type>(arbVoidType())},
            {1, gen::cast<Type>(arbConceptType())},
    });
}

Gen<TypeRef> arbTypeRef() {
    return gen::cast<TypeRef>(arbType());
}

Gen<TypeWithStorage> arbBoolType(EvalMode mode) {
    const int numT = g_dataTypeDecls.size();
    REQUIRE(numT > 0);
    // Find the decl for the 'i8' type
    DeclNode boolDecl;
    for (auto decl : g_dataTypeDecls) {
        if (DeclNode(decl).name() == "i1") {
            boolDecl = DeclNode(decl);
            break;
        }
    }
    return gen::exec([=] () -> TypeWithStorage {
        auto m = mode != modeUnspecified ? mode : *gen::arbitrary<EvalMode>();
        int numRefs = *gen::weightedElement<int>({
                {10, 0},
                {2, 1},
                {1, 2},
                {1, 3},
        });
        TypeWithStorage t = DataType::get(boolDecl, numRefs, m);
        int percentage = *gen::inRange(0, 100);
        if (*gen::inRange(0, 100) < 25) // 25% return MutableType
            t = MutableType::get(t);
        return t;
    });
}

} // namespace TypeFactory
