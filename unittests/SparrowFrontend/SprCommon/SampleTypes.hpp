#pragma once

#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "SparrowFrontend/Utils/cppif/SparrowFrontendTypes.hpp"

using Feather::DataType;
using Nest::CompilationContext;
using SprFrontend::ConceptType;

struct SparrowGeneralFixture;

/**
 * @brief      Set of types to be used by multiple tests
 *
 * Contains the following types:
 *     - i8 (native data type)
 *     - i16 (native data type)
 *     - i32 (native data type)
 *     - FooType (data type)
 *     - BarType (data type)
 *     - NullType (data type)
 *     - Concept1 (concept type)
 *     - Concept2 (concept type)
 *     - ByteType (concept typ)
 *
 * We make the following conventions:
 *     - NullType is registered as the null type
 *     - BarType implicitly converts to FooType
 *     - BarType and FooType models both Concept1 and Concept2
 *     - Concept1 is the base for Concept2
 *     - i8 models ByteType
 *
 * Replaces the overload and convert services from SparrowFrontend to achieve the conventions.
 */
struct SampleTypes {

    enum Flags {
        none = 0,
        onlyNumeric = 1,    //!< Add only the numeric types & concept
        addByteType = 2,    //!< Also add the ByteType concept (by default hidden)
    };

    void init(SparrowGeneralFixture& generalFixture, int flags = none);

    DataType i8Type_;
    DataType i16Type_;
    DataType i32Type_;
    DataType fooType_;
    DataType barType_; // fooType_ -> barType_
    DataType nullType_; // not visible
    DataType typeType_; // not visible
    ConceptType concept1Type_;
    ConceptType concept2Type_;
    ConceptType byteType_;
};
