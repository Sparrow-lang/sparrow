#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Services/IConvertService.h"

#include "Nest/Utils/Tuple.hpp"

#include <utility>

namespace SprFrontend {

//! The Sparrow implementation of the convert service
struct ConvertServiceImpl : IConvertService {

    ConversionResult checkConversion(CompilationContext* context, Type srcType, Type destType,
            ConversionFlags flags = flagsDefault) final;
    ConversionResult checkConversion(
            Node* arg, Type destType, ConversionFlags flags = flagsDefault) final;

private:
    using KeyType = std::tuple<Type, Type, int, const Nest_SourceCode*>;

    //! Cache of all the conversions tried so far
    unordered_map<KeyType, ConversionResult> conversionMap_;

    //! Method that checks for available conversions; use the cache for speeding up search.
    //! It always returns an entry in our cache; therefore it's safe to return by const ref.
    const ConversionResult& cachedCheckConversion(
            CompilationContext* context, int flags, Type srcType, Type destType);

    //! Checks all possible conversions (uncached)
    ConversionResult checkConversionImpl(
            CompilationContext* context, int flags, Type srcType, Type destType);

    //! Checks all possible conversions between types of the same mode
    bool checkConversionSameMode(ConversionResult& res, CompilationContext* context, int flags,
            TypeWithStorage srcType, TypeWithStorage destType);

    //! Checks the conversion to a concept (from data-like or other concept)
    bool checkConversionToConcept(ConversionResult& res, CompilationContext* context, int flags,
            TypeWithStorage srcType, TypeWithStorage destType);

    //! Checks the conversion between data-like types
    bool checkDataConversion(ConversionResult& res, CompilationContext* context, int flags,
            TypeWithStorage srcType, TypeWithStorage destType);

    //! Bring the number of references for the source type to the given value.
    //! Add all the conversions to 'res'. Returns false if conversion is impossible.
    //! The source type must be a data-like type.
    bool adjustReferences(
            ConversionResult& res, TypeWithStorage src, TypeWithStorage dest, bool canAddRef);
};

} // namespace SprFrontend
