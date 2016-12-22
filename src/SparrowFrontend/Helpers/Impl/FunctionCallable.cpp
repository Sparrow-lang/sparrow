#include <StdInc.h>
#include "FunctionCallable.h"
#include <Helpers/SprTypeTraits.h>
#include <Helpers/CommonCode.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/Ct.h>
#include <Helpers/StdDef.h>
#include <Nodes/Builder.h>
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

using namespace SprFrontend;

namespace
{
    Node* impl_injectBackendCode(CompilationContext* context, const Location& loc, const NodeVector& args, EvalMode mode)
    {
        CHECK(loc, args.size() == 1);
        StringRef val = getStringCtValue(args[0]);
        return Feather_mkBackendCode(loc, val, mode);
    }

    Node* impl_typeDescription(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return buildStringLiteral(loc, fromCStr(t->description));
    }

    Node* impl_typeHasStorage(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return buildBoolLiteral(loc, t->hasStorage);
    }

    Node* impl_typeMode(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return buildIntLiteral(loc, t->mode);
    }

    Node* impl_typeCanBeUsedAtCt(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return buildBoolLiteral(loc, t->canBeUsedAtCt);
    }

    Node* impl_typeCanBeUsedAtRt(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return buildBoolLiteral(loc, t->canBeUsedAtRt);
    }

    Node* impl_typeNumRef(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return buildIntLiteral(loc, t->numReferences);
    }

    Node* impl_typeChangeMode(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 2);
        TypeRef t = getType(args[0]);
        int mode = getIntCtValue(args[1]);

        TypeRef res = Feather_checkChangeTypeMode(t, (EvalMode) mode, loc);

        return createTypeNode(context, loc, res);
    }

    Node* impl_typeChangeRefCount(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 2);
        TypeRef t = getType(args[0]);
        int numRef = max(0, getIntCtValue(args[1]));

        TypeRef res = changeRefCount(t, numRef, loc);

        return createTypeNode(context, loc, res);
    }

    Node* impl_typeEQ(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 2);
        TypeRef t1 = getType(args[0]);
        TypeRef t2 = getType(args[1]);

        t1 = Feather_removeLValueIfPresent(t1);
        t2 = Feather_removeLValueIfPresent(t2);

        bool equals = Feather_isSameTypeIgnoreMode(t1, t2);

        // Build a CT value of type bool
        return buildBoolLiteral(loc, equals);
    }

    Node* impl_typeAddRef(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);

        t = Feather_removeLValueIfPresent(t);
        t = changeRefCount(t, t->numReferences+1, loc);
        return createTypeNode(context, loc, t);
    }

    Node* impl_ct(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        TypeRef t = getType(args[0]);

        t = Feather_removeLValueIfPresent(t);
        t = Feather_checkChangeTypeMode(t, modeCt, loc);
        if ( t->mode != modeCt )
            REP_ERROR_RET(nullptr, loc, "Type %1% cannot be used at compile-time") % t;

        return createTypeNode(context, loc, t);
    }

    Node* impl_rt(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        TypeRef t = getType(args[0]);

        t = Feather_removeLValueIfPresent(t);
        t = Feather_checkChangeTypeMode(t, modeRt, loc);
        if ( t->mode != modeRt )
            REP_ERROR_RET(nullptr, loc, "Type %1% cannot be used at run-time") % t;

        return createTypeNode(context, loc, t);
    }

    Node* impl_convertsTo(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 2);
        TypeRef t1 = getType(args[0]);
        TypeRef t2 = getType(args[1]);

        bool result = !!(canConvertType(context, t1, t2));

        return buildBoolLiteral(loc, result);
    }

    Node* impl_staticBuffer(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 1);

        size_t size = getSizeTypeCtValue(args[0]);

        if ( size > numeric_limits<size_t>::max() )
            REP_ERROR_RET(nullptr, loc, "Size of static buffer is too large");

        TypeRef arrType = Feather_getArrayType(StdDef::typeByte, (size_t) size);
        return createTypeNode(context, loc, arrType);
    }

    Node* impl_commonType(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 2);
        TypeRef t1 = getType(args[0]);
        TypeRef t2 = getType(args[1]);

        TypeRef resType = commonType(context, t1, t2);
        return createTypeNode(context, loc, resType);
    }

    Node* impl_Meta_astEval(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 1);

        // Get the impl part of the node
        Node* implPart = mkCompoundExp(loc, args[0], fromCStr("_data"));
        implPart = Feather_mkMemLoad(loc, implPart);    // Remove LValue
        Nest_setContext(implPart, context);
        if ( !Nest_semanticCheck(implPart) )
            return nullptr;

        // Evaluate the handle and get the resulting node
        Node* nodeHandle = (Node*) getByteRefCtValue(implPart);
        if ( !nodeHandle )
            REP_INTERNAL(loc, "Node passed to astEval is invalid");
        return nodeHandle;
    }

    Node* impl_Meta_SourceCode_current(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 0);

        SourceCode* sc = context->sourceCode;
        int* scHandle = reinterpret_cast<int*>(sc);
        Node* base = mkIdentifier(loc, fromCStr("SourceCode"));
        Node* arg = Feather_mkCtValueT(loc, StdDef::typeRefInt, &scHandle);
        return mkFunApplication(loc, base, fromIniList({arg}));
    }

    Node* impl_Meta_CompilationContext_current(CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        CHECK(loc, args.size() == 0);

        int* ctxHandle = reinterpret_cast<int*>(context);
        Node* base = mkIdentifier(loc, fromCStr("CompilationContext"));
        Node* arg = Feather_mkCtValueT(loc, StdDef::typeRefInt, &ctxHandle);
        return mkFunApplication(loc, base, fromIniList({arg}));
    }

    Node* handleIntrinsic(Node* fun, CompilationContext* context, const Location& loc, const NodeVector& args)
    {
        // Check for natives
        const StringRef* nativeName = Nest_getPropertyString(fun, propNativeName);
        if ( nativeName && size(*nativeName) > 0 && nativeName->begin[0] == '$' )
        {
            if ( *nativeName == "$injectBackendCodeRt" )
                return impl_injectBackendCode(context, loc, args, modeRt);
            if ( *nativeName == "$injectBackendCodeCt" )
                return impl_injectBackendCode(context, loc, args, modeCt);
            if ( *nativeName == "$injectBackendCodeRtCt" )
                return impl_injectBackendCode(context, loc, args, modeRtCt);
            if ( *nativeName == "$typeDescription" )
                return impl_typeDescription(context, loc, args);
            if ( *nativeName == "$typeHasStorage" )
                return impl_typeHasStorage(context, loc, args);
            if ( *nativeName == "$typeMode" )
                return impl_typeMode(context, loc, args);
            if ( *nativeName == "$typeCanBeUsedAtCt" )
                return impl_typeCanBeUsedAtCt(context, loc, args);
            if ( *nativeName == "$typeCanBeUsedAtRt" )
                return impl_typeCanBeUsedAtRt(context, loc, args);
            if ( *nativeName == "$typeNumRef" )
                return impl_typeNumRef(context, loc, args);
            if ( *nativeName == "$typeChangeMode" )
                return impl_typeChangeMode(context, loc, args);
            if ( *nativeName == "$typeChangeRefCount" )
                return impl_typeChangeRefCount(context, loc, args);
            if ( *nativeName == "$typeEQ" )
                return impl_typeEQ(context, loc, args);
            if ( *nativeName == "$typeAddRef" )
                return impl_typeAddRef(context, loc, args);
            if ( *nativeName == "$ct" )
                return impl_ct(context, loc, args);
            if ( *nativeName == "$rt" )
                return impl_rt(context, loc, args);
            if ( *nativeName == "$convertsTo" )
                return impl_convertsTo(context, loc, args);
            if ( *nativeName == "$staticBuffer" )
                return impl_staticBuffer(context, loc, args);
            if ( *nativeName == "$commonType" )
                return impl_commonType(context, loc, args);
            if ( *nativeName == "$meta.astEval" )
                return impl_Meta_astEval(context, loc, args);
            if ( *nativeName == "$meta.SourceCode.current" )
                return impl_Meta_SourceCode_current(context, loc, args);
            if ( *nativeName == "$meta.CompilationContext.current" )
                return impl_Meta_CompilationContext_current(context, loc, args);
        }

        return nullptr;
    }
}

FunctionCallable::FunctionCallable(Node* fun)
    : fun_(fun)
    , hasResultParam_(nullptr != getResultParam(fun))
{
}

const Location& FunctionCallable::location() const
{
    return fun_->location;
}

string FunctionCallable::toString() const
{
    return Nest_toString(fun_);
}

size_t FunctionCallable::paramsCount() const
{
    int offset = hasResultParam_ ? 1 : 0;
    return Feather_Function_numParameters(fun_) - offset;
}

Node* FunctionCallable::param(size_t idx) const
{
    int offset = hasResultParam_ ? 1 : 0;
    return Feather_Function_getParameter(fun_, idx+offset);
}

EvalMode FunctionCallable::evalMode() const
{
    return Feather_effectiveEvalMode(fun_);
}
bool FunctionCallable::isAutoCt() const
{
    return Nest_hasProperty(fun_, propAutoCt);
}


Node* FunctionCallable::generateCall(const Location& loc)
{
    ASSERT(context_);
    if ( !Nest_computeType(fun_) )
        return nullptr;

    auto argsCvt = argsWithConversion();

    Node* res = handleIntrinsic(fun_, context_, loc, argsCvt);
    if ( res )
    {
        Nest_setContext(res, context_);
        return res;
    }

    return createFunctionCall(loc, context_, fun_, all(argsCvt));
}

