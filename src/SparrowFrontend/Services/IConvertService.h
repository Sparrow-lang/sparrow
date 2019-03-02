#pragma once

#include "Convert/ConversionResult.h"

namespace SprFrontend {

//! Flags used when checking the conversion to alter the scope of the conversion
enum ConversionFlags {
    flagsDefault = 0,
    flagDontCallConversionCtor = 1,
    flagDontAddReference = 2,
};

//! The interface for the service that deals with conversions between types.
//! Used so that we can easily mock and replace this service
struct IConvertService {
    virtual ~IConvertService() {}

    //! Check if we can convert the source type to the destination type.
    //! The returned object can be used to apply the conversion to nodes of the source type.
    virtual ConversionResult checkConversion(CompilationContext* context, Type srcType,
            Type destType, ConversionFlags flags = flagsDefault) = 0;

    //! Checks if an "argument" node can be converted to a given type
    virtual ConversionResult checkConversion(
            Node* arg, Type destType, ConversionFlags flags = flagsDefault) = 0;
};

//! The convert service instance that we are using across the Sparrow compiler
extern unique_ptr<IConvertService> g_ConvertService;

} // namespace SprFrontend