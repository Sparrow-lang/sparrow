#pragma once

#include "SparrowFrontend/NodeCommonsH.h"

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

//! The type of actions we need to perform for a conversion
enum class ActionType {
    none,
    modeCast,    // direct
    dereference, // direct (implicit for concept dereference)
    bitcast,     // implicit
    makeNull,    // implicit
    addRef,      // implicit
    customCvt,   // custom
};

//! An action for a conversion; action type + destination type
using ConvAction = pair<ActionType, Type>;

//! Class that holds the result of a conversion: both the type of the conversion and the
//! transformation that needs to be applied in order to make the conversion If the conversion only
//! works from a particular source code, also store the source code
class ConversionResult {
public:
    ConversionResult();
    ConversionResult(ConversionType convType);
    ConversionResult(ConversionType convType, ConvAction action,
            const Nest_SourceCode* sourceCode = nullptr);
    //! Chains current conversion with the given 'nextConv'
    ConversionResult(const ConversionResult& nextConv, ConversionType convType, ConvAction action,
            const Nest_SourceCode* sourceCode = nullptr);

    //! Add a conversion at the back of this conversion result
    void addConversion(ConversionType convType, ConvAction action = {},
            const Nest_SourceCode* sourceCode = nullptr);
    void addResult(ConversionResult cvt);

    ConversionType conversionType() const { return convType_; }
    const Nest_SourceCode* sourceCode() const { return sourceCode_; }
    const vector<ConvAction>& convertActions() const { return convertActions_; };

    Node* apply(Node* src) const;
    Node* apply(CompilationContext* context, Node* src) const;

    explicit operator bool() const { return convType_ != convNone; }

private:
    //! The conversion type
    ConversionType convType_{convNone};
    //! The source code needed for the conversion.
    //! If this is nullptr, the conversion can be applied from any source code.
    //! If this is not null, the conversion can be applied only in the given source code.
    const Nest_SourceCode* sourceCode_{nullptr};
    //! The actions needed to convert from one type to the other
    vector<ConvAction> convertActions_{};
};

ostream& operator<<(ostream& os, ConversionType ct);
ostream& operator<<(ostream& os, ActionType act);
ostream& operator<<(ostream& os, const ConversionResult& cvt);

} // namespace SprFrontend
