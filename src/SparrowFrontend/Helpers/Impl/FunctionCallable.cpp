#include <StdInc.h>
#include "FunctionCallable.h"
#include <Helpers/SprTypeTraits.h>
#include <Helpers/CommonCode.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/Ct.h>
#include <Helpers/StdDef.h>
#include <Nodes/Builder.h>
#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;

namespace
{
    DynNode* impl_injectBackendCode(CompilationContext* context, const Location& loc, const DynNodeVector& args, EvalMode mode)
    {
        CHECK(loc, args.size() == 1);
        const char* val = getStringCtValue(args[0]);
        return Feather::mkBackendCode(loc, val, mode);
    }
    
    DynNode* impl_typeDescription(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return mkStringLiteral(loc, t->description);
    }
    
    DynNode* impl_typeHasStorage(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return mkBoolLiteral(loc, t->hasStorage);
    }
    
    DynNode* impl_typeMode(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return mkIntLiteral(loc, t->mode);
    }
    
    DynNode* impl_typeCanBeUsedAtCt(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return mkBoolLiteral(loc, t->canBeUsedAtCt);
    }
    
    DynNode* impl_typeCanBeUsedAtRt(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return mkBoolLiteral(loc, t->canBeUsedAtRt);
    }
    
    DynNode* impl_typeNumRef(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        return mkIntLiteral(loc, t->numReferences);
    }
    
    DynNode* impl_typeChangeMode(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 2);
        TypeRef t = getType(args[0]);
        int mode = getIntCtValue(args[1]);
        
        TypeRef res = changeTypeMode(t, (EvalMode) mode, loc);
        
        return createTypeNode(context, loc, res);
    }
    
    DynNode* impl_typeChangeRefCount(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 2);
        TypeRef t = getType(args[0]);
        int numRef = getIntCtValue(args[1]);
        
        TypeRef res = changeRefCount(t, numRef, loc);
        
        return createTypeNode(context, loc, res);
    }

    DynNode* impl_typeEQ(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 2);
        TypeRef t1 = getType(args[0]);
        TypeRef t2 = getType(args[1]);
        
        t1 = removeLValueIfPresent(t1);
        t2 = removeLValueIfPresent(t2);
        
        bool equals = isSameTypeIgnoreMode(t1, t2);
        
        // Build a CT value of type bool
        return mkBoolLiteral(loc, equals);
    }
    
    DynNode* impl_typeAddRef(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 1);
        TypeRef t = getType(args[0]);
        
        t = removeLValueIfPresent(t);
        t = changeRefCount(t, t->numReferences+1, loc);
        return createTypeNode(context, loc, t);
    }
    
    DynNode* impl_ct(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        TypeRef t = getType(args[0]);
        
        t = removeLValueIfPresent(t);
        t = changeTypeMode(t, modeCt, loc);
        if ( t->mode != modeCt )
            REP_ERROR(loc, "Type %1% cannot be used at compile-time") % t;
        
        return createTypeNode(context, loc, t);
    }
    
    DynNode* impl_rt(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        TypeRef t = getType(args[0]);
        
        t = removeLValueIfPresent(t);
        t = changeTypeMode(t, modeRt, loc);
        if ( t->mode != modeRt )
            REP_ERROR(loc, "Type %1% cannot be used at run-time") % t;
        
        return createTypeNode(context, loc, t);
    }
    
    DynNode* impl_convertsTo(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 2);
        TypeRef t1 = getType(args[0]);
        TypeRef t2 = getType(args[1]);
        
        bool result = !!(canConvertType(context, t1, t2));

        return mkBoolLiteral(loc, result);
    }
    
    DynNode* impl_staticBuffer(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 1);

        int size = getSizeTypeCtValue(args[0]);
        
        if ( size > numeric_limits<size_t>::max() )
            REP_ERROR(loc, "Size of static buffer is too large");
        
        TypeRef arrType = getArrayType(StdDef::typeByte, (size_t) size);
        return createTypeNode(context, loc, arrType);
    }
    
    DynNode* impl_commonType(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 2);
        TypeRef t1 = getType(args[0]);
        TypeRef t2 = getType(args[1]);
        
        TypeRef resType = commonType(context, t1, t2);
        return createTypeNode(context, loc, resType);
    }

    DynNode* impl_Meta_astEval(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 1);

        // Get the impl part of the node
        DynNode* implPart = mkCompoundExp(loc, args[0], "impl");
        implPart = mkMemLoad(loc, implPart);    // Remove LValue
        implPart->setContext(context);
        implPart->semanticCheck();

        // Evaluate the handle and get the resulting node
        DynNode* nodeHandle = (DynNode*) getIntRefCtValue(implPart);
        if ( !nodeHandle )
            REP_INTERNAL(loc, "DynNode passed to astEval is invalid");
        return nodeHandle;
    }
    
    DynNode* impl_Meta_SourceCode_current(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 0);

        SourceCode* sc = context->sourceCode();
        int* scHandle = reinterpret_cast<int*>(sc);
        DynNode* base = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "SourceCode");
        DynNode* arg = mkCtValue(loc, StdDef::typeRefInt, &scHandle);
        return mkFunApplication(loc, base, {arg});
    }

    DynNode* impl_Meta_CompilationContext_current(CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        CHECK(loc, args.size() == 0);

        int* ctxHandle = reinterpret_cast<int*>(context);
        DynNode* base = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "CompilationContext");
        DynNode* arg = mkCtValue(loc, StdDef::typeRefInt, &ctxHandle);
        return mkFunApplication(loc, base, {arg});
    }

    DynNode* handleIntrinsic(Function* fun, CompilationContext* context, const Location& loc, const DynNodeVector& args)
    {
        // Check for natives
        const string* nativeName = fun->getPropertyString(propNativeName);
        if ( nativeName && !nativeName->empty() && (*nativeName)[0] == '$' )
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
            if ( *nativeName == "$Meta.astEval" )
                return impl_Meta_astEval(context, loc, args);
            if ( *nativeName == "$Meta.SourceCode.current" )
                return impl_Meta_SourceCode_current(context, loc, args);
            if ( *nativeName == "$Meta.CompilationContext.current" )
                return impl_Meta_CompilationContext_current(context, loc, args);
        }

        return nullptr;
    }
}

FunctionCallable::FunctionCallable(Function* fun)
    : fun_(fun)
    , hasResultParam_(nullptr != getResultParam(fun))
{
}

const Location& FunctionCallable::location() const
{
    return fun_->location();
}

string FunctionCallable::toString() const
{
    return fun_->toString();
}

size_t FunctionCallable::paramsCount() const
{
    int offset = hasResultParam_ ? 1 : 0;
    return fun_->numParameters() - offset;
}

DynNode* FunctionCallable::param(size_t idx) const
{
    int offset = hasResultParam_ ? 1 : 0;
    return fun_->getParameter(idx+offset);
}

EvalMode FunctionCallable::evalMode() const
{
    return effectiveEvalMode(fun_);
}
bool FunctionCallable::isAutoCt() const
{
    return fun_->hasProperty(propAutoCt);
}


DynNode* FunctionCallable::generateCall(const Location& loc)
{
    ASSERT(context_);
    fun_->computeType();

    auto argsCvt = argsWithConversion();
    
    DynNode* res = handleIntrinsic(fun_, context_, loc, argsCvt);
    if ( res )
    {
        res->setContext(context_);
        return res;
    }

    return createFunctionCall(loc, context_, fun_, argsCvt);
}

