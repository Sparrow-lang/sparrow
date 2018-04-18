#pragma once

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.h"
#include "Nest/Api/EvalMode.h"

namespace TypeFactory {

using rc::Gen;
using namespace rc;

//! The declarations for all the datatypes that we will use to generate data types and derived types
extern vector<Node*> g_dataTypeDecls;

//! Returns a generator for Void types with arbitrary mode (there are 2 types of Void)
Gen<TypeRef> arbVoidType();

//! Returns a generator for arbitrary data types.
//! We would generate types with decls found in g_dataTypeDecls
Gen<TypeRef> arbDataType(EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator for arbitrary LValue types
Gen<TypeRef> arbLValueType(EvalMode mode = modeUnspecified, int minRef = 1, int maxRef = 4);

//! Returns a generator for arbitrary array types
Gen<TypeRef> arbArrayType(EvalMode mode = modeUnspecified);

//! Returns a generator for arbitrary function types
Gen<TypeRef> arbFunctionType(EvalMode mode = modeUnspecified);

//! Returns a generator of types with storage
Gen<TypeRef> arbTypeWithStorage(EvalMode mode, int minRef, int maxRef);

//! Returns a generator for arbitrary basic storage types (data type and LValue type)
Gen<TypeRef> arbBasicStorageType(EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator for arbitrary types (or all kinds)
Gen<TypeRef> arbType();

} // namespace TypeFactory

namespace rc {

//! Helper to make RapidCheck generate arbitrary types
template <> struct Arbitrary<TypeRef> {
    static Gen<TypeRef> arbitrary() { return TypeFactory::arbType(); }
};

} // namespace rc
