#include <StdInc.h>
#include "Intrinsics.h"
#include <Helpers/SprTypeTraits.h>
#include <Helpers/CommonCode.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/Ct.h>
#include <Helpers/StdDef.h>
#include <Helpers/Convert.h>
#include <Nodes/Builder.h>
#include <Nodes/Exp.hpp>
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"

using namespace Feather;

namespace {

NodeHandle impl_injectBackendCode(
        CompilationContext* context, const Location& loc, NodeRange args, EvalMode mode) {
    CHECK(loc, args.size() == 1);
    StringRef val = getStringCtValue(args[0]);
    return Feather::BackendCode::create(loc, val, mode);
}

NodeHandle impl_typeDescription(
        CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 1);
    Type t = getType(args[0]);
    return buildStringLiteral(loc, t.description());
}

NodeHandle impl_typeHasStorage(
        CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 1);
    Type t = getType(args[0]);
    return buildBoolLiteral(loc, t.hasStorage());
}

NodeHandle impl_typeMode(CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 1);
    Type t = getType(args[0]);
    return buildIntLiteral(loc, t.mode());
}

NodeHandle impl_typeCanBeUsedAtRt(
        CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 1);
    Type t = getType(args[0]);
    return buildBoolLiteral(loc, t.canBeUsedAtRt());
}

NodeHandle impl_typeNumRef(CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 1);
    Type t = getType(args[0]);
    return buildIntLiteral(loc, t.numReferences());
}

NodeHandle impl_typeChangeMode(
        CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 2);
    Type t = getType(args[0]);
    int mode = getIntCtValue(args[1]);

    Type res = t.changeMode((EvalMode)mode, loc);

    return createTypeNode(context, loc, res);
}

NodeHandle impl_typeChangeRefCount(
        CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 2);
    Type t = getType(args[0]);
    int numRef = max(0, getIntCtValue(args[1]));

    Type res = changeRefCount(t, numRef, loc);

    return createTypeNode(context, loc, res);
}

NodeHandle impl_typeEQ(CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 2);
    Type t1 = getType(args[0]);
    Type t2 = getType(args[1]);

    t1 = Feather::removeCategoryIfPresent(t1);
    t2 = Feather::removeCategoryIfPresent(t2);

    bool equals = Nest::sameTypeIgnoreMode(t1, t2);

    // Build a CT value of type bool
    return buildBoolLiteral(loc, equals);
}

NodeHandle impl_typeAddRef(CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 1);
    Type t = getType(args[0]);

    t = Feather::removeCategoryIfPresent(t);
    t = changeRefCount(t, t.numReferences() + 1, loc);
    return createTypeNode(context, loc, t);
}

NodeHandle impl_ct(CompilationContext* context, const Location& loc, NodeRange args) {
    Type t = getType(args[0]);

    t = Feather::removeCategoryIfPresent(t);
    t = Type(t).changeMode(modeCt, loc);
    if (t.mode() != modeCt)
        REP_ERROR_RET(nullptr, loc, "Type %1% cannot be used at compile-time") % t;

    return createTypeNode(context, loc, t);
}

NodeHandle impl_rt(CompilationContext* context, const Location& loc, NodeRange args) {
    Type t = getType(args[0]);

    t = Feather::removeCategoryIfPresent(t);
    t = Type(t).changeMode(modeRt, loc);
    if (t.mode() != modeRt)
        REP_ERROR_RET(nullptr, loc, "Type %1% cannot be used at run-time") % t;

    return createTypeNode(context, loc, t);
}

NodeHandle impl_convertsTo(CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 2);
    Type t1 = getType(args[0]);
    Type t2 = getType(args[1]);

    bool result = !!(g_ConvertService->checkConversion(context, t1, t2));

    return buildBoolLiteral(loc, result);
}

NodeHandle impl_staticBuffer(CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 1);

    size_t size = getSizeTypeCtValue(args[0]);

    if (size > numeric_limits<size_t>::max())
        REP_ERROR_RET(nullptr, loc, "Size of static buffer is too large");

    Type arrType = Feather_getArrayType(StdDef::typeByte, (size_t)size);
    return createTypeNode(context, loc, arrType);
}

NodeHandle impl_commonType(CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 2);
    Type t1 = getType(args[0]);
    Type t2 = getType(args[1]);

    Type resType = commonType(context, t1, t2);
    return createTypeNode(context, loc, resType);
}

NodeHandle impl_Meta_astEval(CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 1);

    // Get the impl part of the node
    NodeHandle implPart = CompoundExp::create(loc, args[0], "data");
    implPart = Feather::MemLoadExp::create(loc, implPart); // Remove category type
    implPart.setContext(context);
    if (!implPart.semanticCheck())
        return nullptr;

    // Evaluate the handle and get the resulting node
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* nodeHandle = (Node*)getByteRefCtValue(implPart);
    if (!nodeHandle)
        REP_INTERNAL(loc, "Node passed to astEval is invalid");
    return nodeHandle;
}

NodeHandle impl_Meta_SourceCode_current(
        CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 0);

    return buildLiteral(loc, "SourceCode", context->sourceCode);
}

NodeHandle impl_Meta_CompilationContext_current(
        CompilationContext* context, const Location& loc, NodeRange args) {
    CHECK(loc, args.size() == 0);

    return buildLiteral(loc, "CompilationContext", context);
}
} // namespace

NodeHandle SprFrontend::handleIntrinsic(Feather::FunctionDecl fun, CompilationContext* context,
        const Location& loc, NodeRange args) {
    // Check for natives
    StringRef nativeName = fun.getPropertyStringDeref(propNativeName);
    if (nativeName && nativeName.begin[0] == '$') {
        if (nativeName == "$injectBackendCode")
            return impl_injectBackendCode(context, loc, args, modeRt);
        if (nativeName == "$injectBackendCodeCt")
            return impl_injectBackendCode(context, loc, args, modeCt);
        if (nativeName == "$typeDescription")
            return impl_typeDescription(context, loc, args);
        if (nativeName == "$typeHasStorage")
            return impl_typeHasStorage(context, loc, args);
        if (nativeName == "$typeMode")
            return impl_typeMode(context, loc, args);
        if (nativeName == "$typeCanBeUsedAtRt")
            return impl_typeCanBeUsedAtRt(context, loc, args);
        if (nativeName == "$typeNumRef")
            return impl_typeNumRef(context, loc, args);
        if (nativeName == "$typeChangeMode")
            return impl_typeChangeMode(context, loc, args);
        if (nativeName == "$typeChangeRefCount")
            return impl_typeChangeRefCount(context, loc, args);
        if (nativeName == "$typeEQ")
            return impl_typeEQ(context, loc, args);
        if (nativeName == "$typeAddRef")
            return impl_typeAddRef(context, loc, args);
        if (nativeName == "$ct")
            return impl_ct(context, loc, args);
        if (nativeName == "$rt")
            return impl_rt(context, loc, args);
        if (nativeName == "$convertsTo")
            return impl_convertsTo(context, loc, args);
        if (nativeName == "$staticBuffer")
            return impl_staticBuffer(context, loc, args);
        if (nativeName == "$commonType")
            return impl_commonType(context, loc, args);
        if (nativeName == "$meta.astEval")
            return impl_Meta_astEval(context, loc, args);
        if (nativeName == "$meta.SourceCode.current")
            return impl_Meta_SourceCode_current(context, loc, args);
        if (nativeName == "$meta.CompilationContext.current")
            return impl_Meta_CompilationContext_current(context, loc, args);
    }

    return nullptr;
}
