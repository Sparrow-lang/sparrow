#pragma once

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.h"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "SparrowFrontend/Utils/cppif/SparrowFrontendTypes.hpp"
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

//! Returns a generator for arbitrary Const types
Gen<Feather::ConstType> arbConstType(
        EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator for arbitrary Mutable types
Gen<Feather::MutableType> arbMutableType(
        EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator for arbitrary Temp types
Gen<Feather::TempType> arbTempType(EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator for arbitrary array types
Gen<Feather::ArrayType> arbArrayType(EvalMode mode = modeUnspecified);

//! Returns a generator for arbitrary function types
Gen<Feather::FunctionType> arbFunctionType(
        EvalMode mode = modeUnspecified, Nest::TypeWithStorage resType = {});

//! Returns a generator for arbitrary concept types
Gen<SprFrontend::ConceptType> arbConceptType(EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator of types with storage
Gen<Nest::TypeWithStorage> arbTypeWithStorage(
        EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator for arbitrary basic storage types (data-like type)
Gen<Nest::TypeWithStorage> arbBasicStorageType(
        EvalMode mode = modeUnspecified, int minRef = 0, int maxRef = 4);

//! Returns a generator for arbitrary types (or all kinds)
Gen<Nest::Type> arbType();

//! Returns an arbitrary type, as a TypeRef
Gen<Nest::TypeRef> arbTypeRef();

//! Returns a generator for Bool types; it will generate DataType or MutableType for a struct that
//! has 'i8' native name
Gen<Nest::TypeWithStorage> arbBoolType(EvalMode mode = modeUnspecified);

} // namespace TypeFactory

namespace rc {

using Nest::TypeRef;

template <> struct Arbitrary<Feather::VoidType> {
    static Gen<Feather::VoidType> arbitrary() { return TypeFactory::arbVoidType(); }
};
template <> struct Arbitrary<Feather::DataType> {
    static Gen<Feather::DataType> arbitrary() { return TypeFactory::arbDataType(); }
};
template <> struct Arbitrary<Feather::ConstType> {
    static Gen<Feather::ConstType> arbitrary() { return TypeFactory::arbConstType(); }
};
template <> struct Arbitrary<Feather::MutableType> {
    static Gen<Feather::MutableType> arbitrary() { return TypeFactory::arbMutableType(); }
};
template <> struct Arbitrary<Feather::TempType> {
    static Gen<Feather::TempType> arbitrary() { return TypeFactory::arbTempType(); }
};
template <> struct Arbitrary<Feather::ArrayType> {
    static Gen<Feather::ArrayType> arbitrary() { return TypeFactory::arbArrayType(); }
};
template <> struct Arbitrary<Feather::FunctionType> {
    static Gen<Feather::FunctionType> arbitrary() { return TypeFactory::arbFunctionType(); }
};
template <> struct Arbitrary<SprFrontend::ConceptType> {
    static Gen<SprFrontend::ConceptType> arbitrary() { return TypeFactory::arbConceptType(); }
};
template <> struct Arbitrary<Nest::TypeWithStorage> {
    static Gen<Nest::TypeWithStorage> arbitrary() { return TypeFactory::arbTypeWithStorage(); }
};
template <> struct Arbitrary<Nest::Type> {
    static Gen<Nest::Type> arbitrary() { return TypeFactory::arbType(); }
};
template <> struct Arbitrary<TypeRef> {
    static Gen<TypeRef> arbitrary() { return TypeFactory::arbTypeRef(); }
};

} // namespace rc
