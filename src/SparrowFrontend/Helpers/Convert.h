#pragma once

#include "SparrowFrontend/NodeCommonsH.h"

#include <functional>

namespace SprFrontend {
//! The type of conversion that can be applied between two types
enum ConversionType {
    convNone = 0,
    convCustom,
    convConceptWithImplicit,
    convConcept,
    convImplicit,
    convDirect,
};
ConversionType combine(ConversionType lhs, ConversionType rhs);
ConversionType worstConv(ConversionType lhs, ConversionType rhs);
ConversionType bestConv(ConversionType lhs, ConversionType rhs);

//! Type of function that can be applied to convert a node into another node
typedef function<Node*(Node*)> ConversionFun;

//! Class that holds the result of a conversion: both the type of the conversion and the
//! transformation that needs to be applied in order to make the conversion If the conversion only
//! works from a particular source code, also store the source code
class ConversionResult {
public:
    ConversionResult(ConversionType convType, ConversionFun fun = ConversionFun(),
            const SourceCode* sourceCode = nullptr);

    ConversionType conversionType() const { return convType_; }
    ConversionFun conversionFun() const { return convFun_; }
    const SourceCode* sourceCode() const { return sourceCode_; }

    Node* apply(Node* src) const;
    Node* apply(CompilationContext* context, Node* src) const;

    explicit operator bool() const { return convType_ != convNone; }

private:
    ConversionType convType_;
    ConversionFun convFun_;
    const SourceCode* sourceCode_;
};

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

    //! Check if we can convert the source type to the destination type; also check what kind of
    //! conversion is needed
    virtual ConversionResult canConvertType(CompilationContext* context, TypeRef srcType,
            TypeRef destType, ConversionFlags flags = flagsDefault) = 0;

    //! Checks if an "argument" node can be converted to a given type
    virtual ConversionResult canConvert(
            Node* arg, TypeRef destType, ConversionFlags flags = flagsDefault) = 0;
};

//! Implementation of the convert service
struct ConvertService : IConvertService {
    ConversionResult canConvertType(CompilationContext* context, TypeRef srcType, TypeRef destType,
            ConversionFlags flags = flagsDefault) final;
    ConversionResult canConvert(
            Node* arg, TypeRef destType, ConversionFlags flags = flagsDefault) final;
};

//! The convert service instance that we are using across the Sparrow compiler
extern IConvertService* g_ConvertService;

} // namespace SprFrontend
