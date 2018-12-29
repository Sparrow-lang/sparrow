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
 *
 * We make the following conventions:
 *     - NullType is registered as the null type
 *     - Concept1 is the base for Concept2
 *     - BarType implicitly converts to FooType
 *
 * Replaces the overload and convert services from SparrowFrontend to achieve the conventions.
 */
struct SampleTypes {

    void init(SparrowGeneralFixture& generalFixture);

    DataType fooType_;
    DataType barType_; // fooType_ -> barType_
    DataType nullType_;
    ConceptType concept1Type_;
    ConceptType concept2Type_;
};
