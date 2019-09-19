#include "StdInc.h"
#include "TypeFactory.hpp"
#include "RcBasic.hpp"

#include "Nest/Api/Type.h"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"
#include "SparrowFrontend/SparrowFrontendTypes.hpp"

namespace TypeFactory {

using namespace rc;
using namespace Feather;
using SprFrontend::ConceptType;

vector<NodeHandle> g_dataTypeDecls{};
vector<NodeHandle> g_conceptDecls{};

namespace {
//! Generate an eval mode if it's unspecified.
//! To be called only from gen::exec contexts.
EvalMode genModeIfNotSpecified(EvalMode mode) {
    return mode == modeUnspecified ? *gen::arbitrary<EvalMode>() : mode;
}
} // namespace

Gen<VoidType> arbVoidType() { return gen::apply(&VoidType::get, gen::arbitrary<EvalMode>()); }

Gen<DataType> arbDataType(EvalMode mode) {
    return gen::exec([=]() -> DataType {
        int idx = *gen::inRange(0, int(g_dataTypeDecls.size()));
        EvalMode genMode = genModeIfNotSpecified(mode);
        REQUIRE(idx < g_dataTypeDecls.size());
        return DataType::get(g_dataTypeDecls[idx], genMode);
    });
}

Gen<PtrType> arbPtrType(EvalMode mode) {
    return gen::exec([=]() -> PtrType {
        // TODO: allow ptr types to be based on category types
        // auto base = *arbTypeWeighted(mode, 10, 2, 3, 3, 1);
        auto base = *arbTypeWeighted(mode, 10, 2, 0, 0, 0);
        return PtrType::get(base);
    });
}

Gen<ConstType> arbConstType(EvalMode mode) {
    return gen::exec([=]() -> ConstType {
        auto base = *arbTypeWeighted(mode, 4, 2, 0, 0, 0);
        return ConstType::get(base);
    });
}

Gen<MutableType> arbMutableType(EvalMode mode) {
    return gen::exec([=]() -> MutableType {
        auto base = *arbTypeWeighted(mode, 4, 2, 0, 0, 0);
        return MutableType::get(base);
    });
}

Gen<TempType> arbTempType(EvalMode mode) {
    return gen::exec([=]() -> TempType {
        auto base = *arbTypeWeighted(mode, 4, 2, 0, 0, 0);
        return TempType::get(base);
    });
}

Gen<ArrayType> arbArrayType(EvalMode mode) {
    return gen::exec([=]() -> ArrayType {
        auto unit = *arbTypeWeighted(mode, 4, 2, 0, 0, 0);
        auto count = *gen::inRange(1, 100);
        return ArrayType::get(unit, count);
    });
}

Gen<FunctionType> arbFunctionType(EvalMode mode, Nest::TypeWithStorage resType) {
    return gen::exec([=]() -> FunctionType {
        EvalMode genMode = genModeIfNotSpecified(mode);
        int numTypes = *gen::inRange(1, 5);
        Nest::TypeRef types[5];
        for (int i = 0; i < numTypes; i++) {
            auto t = i == 0 && resType ? resType : *arbDataOrPtrType(genMode);
            types[i] = t;
        }
        return FunctionType::get(&types[0], numTypes, genMode);
    });
}

Gen<ConceptType> arbConceptType(EvalMode mode, int minRef, int maxRef) {
    return gen::exec([=]() -> ConceptType {
        const int numT = g_conceptDecls.size();
        REQUIRE(numT > 0);
        auto idx = *gen::inRange(0, numT);
        REQUIRE(idx < g_conceptDecls.size());
        auto numReferences = *gen::inRange(minRef, maxRef);
        EvalMode genMode = genModeIfNotSpecified(mode);
        return ConceptType::get(g_conceptDecls[idx], numReferences, genMode);
    });
}

Gen<TypeWithStorage> arbTypeWeighted(EvalMode mode, int weightDataType, int weightPtrType,
        int weightConstType, int weightMutableType, int weightTempType, int weightArrayType,
        int weightFunctionType, int weightConceptType) {
    return gen::weightedOneOf<TypeWithStorage>({
            {weightDataType, gen::cast<TypeWithStorage>(arbDataType(mode))},
            {weightPtrType, gen::cast<TypeWithStorage>(arbPtrType(mode))},
            {weightConstType, gen::cast<TypeWithStorage>(arbConstType(mode))},
            {weightMutableType, gen::cast<TypeWithStorage>(arbMutableType(mode))},
            {weightTempType, gen::cast<TypeWithStorage>(arbTempType(mode))},
            {weightArrayType, gen::cast<TypeWithStorage>(arbArrayType(mode))},
            {weightFunctionType, gen::cast<TypeWithStorage>(arbFunctionType(mode))},
            {weightConceptType, gen::cast<TypeWithStorage>(arbConceptType(mode))},
    });
}

Gen<Feather::TypeWithStorage> arbDataOrPtrType(EvalMode mode) {
    return arbTypeWeighted(mode, 3, 2, 0, 0, 0, 0, 0, 0);
}

Gen<TypeWithStorage> arbTypeWithStorage(EvalMode mode) {
    return arbTypeWeighted(mode, 5, 2, 3, 3, 1, 1, 1, 0);
    // TODO: concepts
}

Gen<TypeWithStorage> arbBasicStorageType(EvalMode mode) { return arbTypeWeighted(mode); }

Gen<TypeWithStorage> arbTypeWithRef(EvalMode mode) { return arbTypeWeighted(mode, 0, 2, 3, 3, 1); }

Gen<Type> arbType() {
    return gen::weightedOneOf<Type>({
            {17, gen::cast<Type>(arbTypeWeighted(modeUnspecified, 5, 2, 3, 3, 1, 1, 1, 1))},
            {1, gen::cast<Type>(arbVoidType())},
    });
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
    return gen::exec([=]() -> TypeWithStorage {
        // underlying data type
        EvalMode genMode = genModeIfNotSpecified(mode);
        TypeWithStorage t = Feather::DataType::get(boolDecl, genMode);
        // ptr type?
        if (*gen::inRange(0, 100) < 25) // 25% return PtrType
            t = PtrType::get(t);
        // const or mutable?
        if (*gen::inRange(0, 100) < 25) // 25% return ConstType
            t = ConstType::get(t);
        else if (*gen::inRange(0, 100) < 25) // 25% return MutableType
            t = MutableType::get(t);
        return t;
    });
}

} // namespace TypeFactory
