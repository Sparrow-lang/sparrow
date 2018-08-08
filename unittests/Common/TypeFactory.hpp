#pragma once

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.h"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Nest/Api/EvalMode.h"
#include "Nest/Utils/cppif/Fwd.hpp"

namespace TypeFactory {

using rc::Gen;
using namespace rc;
using Nest::Node;
using Nest::NodeHandle;
using Nest::TypeRef;

//! The declarations for all the datatypes that we will use to generate data types and derived types
extern vector<NodeHandle> g_dataTypeDecls;

//! The declarations for all the concepts/generic datatypes nodes, used for creating concept types
extern vector<NodeHandle> g_conceptDecls;

//! Returns a generator for Void types with arbitrary mode (there are 2 types of Void)
Gen<Feather::VoidType> arbVoidType();

//! Returns a generator for arbitrary data types.
//! We would generate types with decls found in g_dataTypeDecls
Gen<Feather::DataType> arbDataType(EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator for arbitrary LValue types
Gen<Feather::LValueType> arbLValueType(
        EvalMode mode = modeUnspecified, int minRef = 1, int maxRef = 4);

//! Returns a generator for arbitrary array types
Gen<Feather::ArrayType> arbArrayType(EvalMode mode = modeUnspecified);

//! Returns a generator for arbitrary function types
Gen<Feather::FunctionType> arbFunctionType(EvalMode mode = modeUnspecified);

//! Returns a generator for arbitrary concept types
Gen<Feather::TypeBase> arbConceptType(
        EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator of types with storage
Gen<Feather::TypeWithStorage> arbTypeWithStorage(
        EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator for arbitrary basic storage types (data type and LValue type)
Gen<Feather::TypeWithStorage> arbBasicStorageType(
        EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator for arbitrary types (or all kinds)
Gen<Feather::TypeBase> arbType();

//! Returns an arbitrary type, as a TypeRef
Gen<Nest::TypeRef> arbTypeRef();

} // namespace TypeFactory

namespace rc {

using Nest::TypeRef;

template <> struct Arbitrary<Feather::VoidType> {
    static Gen<Feather::VoidType> arbitrary() { return TypeFactory::arbVoidType(); }
};
template <> struct Arbitrary<Feather::DataType> {
    static Gen<Feather::DataType> arbitrary() { return TypeFactory::arbDataType(); }
};
template <> struct Arbitrary<Feather::LValueType> {
    static Gen<Feather::LValueType> arbitrary() { return TypeFactory::arbLValueType(); }
};
template <> struct Arbitrary<Feather::ArrayType> {
    static Gen<Feather::ArrayType> arbitrary() { return TypeFactory::arbArrayType(); }
};
template <> struct Arbitrary<Feather::FunctionType> {
    static Gen<Feather::FunctionType> arbitrary() { return TypeFactory::arbFunctionType(); }
};
template <> struct Arbitrary<Feather::TypeWithStorage> {
    static Gen<Feather::TypeWithStorage> arbitrary() { return TypeFactory::arbTypeWithStorage(); }
};
template <> struct Arbitrary<Feather::TypeBase> {
    static Gen<Feather::TypeBase> arbitrary() { return TypeFactory::arbType(); }
};
template <> struct Arbitrary<TypeRef> {
    static Gen<TypeRef> arbitrary() { return TypeFactory::arbTypeRef(); }
};

} // namespace rc
