#include <StdInc.h>
#include "SparrowFrontend/Services/Convert/ConversionResult.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"

#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/NodeCommonsCpp.h"
#include "SparrowFrontend/SparrowFrontendTypes.hpp"
#include "SparrowFrontend/Nodes/SprProperties.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Utils/Tuple.hpp"

#include <utility>

namespace SprFrontend {

using Feather::removeCatOrRef;
using Nest::TypeWithStorage;

ConversionType combine(ConversionType lhs, ConversionType rhs) {
    if (lhs == convConcept && rhs == convImplicit)
        return convConceptWithImplicit;
    if (rhs == convConcept && lhs == convImplicit)
        return convConceptWithImplicit;
    return worstConv(lhs, rhs);
}

ConversionType worstConv(ConversionType lhs, ConversionType rhs) {
    return (ConversionType)min(lhs, rhs);
}

ConversionType bestConv(ConversionType lhs, ConversionType rhs) {
    return (ConversionType)max(lhs, rhs);
}

ConversionResult::ConversionResult() {}

ConversionResult::ConversionResult(ConversionType convType)
    : convType_(convType) {}

ConversionResult::ConversionResult(
        ConversionType convType, ConvAction action, const Nest_SourceCode* sourceCode)
    : convType_(convType)
    , sourceCode_(sourceCode) {

    if (convType != convNone && action.first != ActionType::none)
        convertActions_.push_back(action);
}
ConversionResult::ConversionResult(const ConversionResult& nextConv, ConversionType convType,
        ConvAction action, const Nest_SourceCode* sourceCode)
    : convType_(combine(convType, nextConv.conversionType()))
    , sourceCode_(sourceCode) {

    if (convType != convNone) {
        // Check if sourceCode matches
        if (sourceCode) {
            if (nextConv.sourceCode() && nextConv.sourceCode() != sourceCode) {
                convType_ = convNone;
                return;
            }
        } else
            sourceCode_ = nextConv.sourceCode();

        // Combine the actions
        convertActions_.reserve(nextConv.convertActions_.size() + 1);
        if (action.first != ActionType::none)
            convertActions_.emplace_back(action);
        convertActions_.insert(convertActions_.end(), nextConv.convertActions_.begin(),
                nextConv.convertActions_.end());
    }
}

void ConversionResult::addConversion(
        ConversionType convType, ConvAction action, const Nest_SourceCode* sourceCode) {
    // Combine the conversion types
    convType_ = combine(convType_, convType);

    if (convType != convNone) {
        // Check if sourceCode matches
        if (sourceCode) {
            if (sourceCode_ && sourceCode_ != sourceCode) {
                convType_ = convNone;
                return;
            }
        }

        // Add the action to the back
        if (action.first != ActionType::none)
            convertActions_.emplace_back(action);
    }
}

void ConversionResult::addResult(ConversionResult cvt) {
    if (cvt.sourceCode()) {
        if (sourceCode_ && sourceCode_ != cvt.sourceCode()) {
            convType_ = convNone;
            return;
        }
    }
    convType_ = combine(convType_, cvt.conversionType());
    const auto& nextAct = cvt.convertActions();
    convertActions_.insert(convertActions_.end(), nextAct.begin(), nextAct.end());
}

//! Apply one conversion action to the given node
Node* applyOnce(Node* src, ConvAction action) {
    Type destT = action.second;
    switch (action.first) {
    case ActionType::none:
        return src;
    case ActionType::modeCast:
        return src; // Nothing to do when changing mode
    case ActionType::dereference:
        return Feather_mkMemLoad(src->location, src);
    case ActionType::bitcast:
        return Feather_mkBitcast(src->location, Feather_mkTypeNode(src->location, destT), src);
    case ActionType::makeNull:
        return Feather_mkNull(src->location, Feather_mkTypeNode(src->location, destT));
    case ActionType::addRef: {
        Type srcT = removeCatOrRef(TypeWithStorage(destT));
        Node* var = Feather_mkVar(
                src->location, StringRef("$tmpForRef"), Feather_mkTypeNode(src->location, srcT));
        Node* varRef = Feather_mkVarRef(src->location, var);
        Node* store = Feather_mkMemStore(src->location, src, varRef);
        Node* cast =
                Feather_mkBitcast(src->location, Feather_mkTypeNode(src->location, destT), varRef);
        return Feather_mkNodeList(src->location, fromIniList({var, store, cast}));
    }
    case ActionType::customCvt: {
        EvalMode destMode = destT.mode();
        Node* destClass = destT.referredNode();
        Node* refToClass = createTypeNode(
                src->context, src->location, Feather_getDataType(destClass, 0, modeRt));
        return Feather_mkChangeMode(src->location,
                mkFunApplication(src->location, refToClass, fromIniList({src})), destMode);
    }
    }
}

Node* ConversionResult::apply(Node* src) const {
    // If there is no conversion, return the original node
    if (convType_ == convNone)
        return src;

    auto ctx = src->context;

    // If there is a series of conversions, apply them in chain
    for (ConvAction action : convertActions_) {
        ASSERT(src);
        src = applyOnce(src, action);
    }
    // If the original node had a context, set it to the result
    if (ctx)
        Nest_setContext(src, ctx);
    return src;
}

Node* ConversionResult::apply(CompilationContext* context, Node* src) const {
    auto res = apply(src);
    Nest_setContext(res, context);
    return res;
}

ostream& operator<<(ostream& os, ConversionType ct) {
    switch (ct) {
    case convNone:
        os << "None";
        break;
    case convCustom:
        os << "Custom";
        break;
    case convConceptWithImplicit:
        os << "ConceptWithImplicit";
        break;
    case convConcept:
        os << "Concept";
        break;
    case convImplicit:
        os << "Implicit";
        break;
    case convDirect:
        os << "Direct";
        break;
    }
    return os;
}
ostream& operator<<(ostream& os, ActionType act) {
    switch (act) {
    case ActionType::none:
        os << "none";
        break;
    case ActionType::modeCast:
        os << "modeCast";
        break;
    case ActionType::dereference:
        os << "dereference";
        break;
    case ActionType::bitcast:
        os << "bitcast";
        break;
    case ActionType::makeNull:
        os << "makeNull";
        break;
    case ActionType::addRef:
        os << "addRef";
        break;
    case ActionType::customCvt:
        os << "customCvt";
        break;
    default:
        os << "?";
    }
    return os;
}
ostream& operator<<(ostream& os, const ConversionResult& cvt) {
    os << cvt.conversionType() << " - [";
    bool first = true;
    for (auto act : cvt.convertActions()) {
        if (first)
            first = false;
        else
            os << " + ";
        os << act.first << "(" << act.second << ")";
    }
    os << "]";
    return os;
}
} // namespace SprFrontend
