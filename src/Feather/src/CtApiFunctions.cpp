#include "Feather/src/StdInc.h"
#include "CtApiFunctions.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Api/Node.h"
#include "Nest/Api/Backend.h"
#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/StringRef.hpp"

using namespace Nest;

namespace
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Diagnostic
    //
    void ctApi_SourceCode_fromFilename(const SourceCode** sret, StringRef filename)
    {
        *sret = Nest_getSourceCodeForFilename(filename);
    }
    void ctApi_SourceCode_filename(StringRef* sret, SourceCode** thisArg)
    {
        *sret = fromCStr((*thisArg)->url);
    }

    void ctApi_Location_getCorrespondingCode(StringRef* sret, Location* thisArg)
    {
        const SourceCode* sourceCode = (const SourceCode*) thisArg->sourceCode;
        string code;
        StringRef lineStr = { 0, 0 };
        if ( sourceCode && !(thisArg->start.line == thisArg->end.line && thisArg->start.col > thisArg->end.col) )
        {
            lineStr = Nest_getSourceCodeLine(sourceCode, thisArg->start.line);
            size_t len = lineStr.end - lineStr.begin;
            size_t count = thisArg->end.line == thisArg->start.line
                            ? thisArg->end.col - thisArg->start.col
                            : len - thisArg->start.col; // first line until the end
            lineStr.begin += thisArg->start.col-1;
            lineStr.end = lineStr.begin + count;
        }
        *sret = lineStr;
    }

    void ctApi_report(int type, StringRef message, Location* location)
    {
        Location loc = location ? *location : Location();
        Nest_reportDiagnostic(loc, (DiagnosticSeverity) type, message.begin);
    }
    void ctApi_raise()
    {
        REP_INFO(NOLOC, "CT API invoked raise. Exiting...");
        exit(0);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CompilationContext
    //
    int ctApi_CompilationContext_evalMode(CompilationContext** thisArg)
    {
        return (*thisArg)->evalMode;
    }

    void ctApi_CompilationContext_sourceCode(SourceCode** sret, CompilationContext** thisArg)
    {
        *sret = (*thisArg)->sourceCode;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // AstType
    //
    int ctApi_AstType_typeKind(TypeRef* thisArg)
    {
        return (*thisArg)->typeKind;
    }
    void ctApi_AstType_toString(StringRef* sret, TypeRef* thisArg)
    {
        *sret = dupCStr((*thisArg)->description);
    }
    bool ctApi_AstType_hasStorage(TypeRef* thisArg)
    {
        return (*thisArg)->hasStorage;
    }
    int ctApi_AstType_numReferences(TypeRef* thisArg)
    {
        return (*thisArg)->numReferences;
    }
    int ctApi_AstType_mode(TypeRef* thisArg)
    {
        return (*thisArg)->mode;
    }
    bool ctApi_AstType_canBeUsedAtCt(TypeRef* thisArg)
    {
        return (*thisArg)->canBeUsedAtCt;
    }
    bool ctApi_AstType_canBeUsedAtRt(TypeRef* thisArg)
    {
        return (*thisArg)->canBeUsedAtRt;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // AstNode
    //
    void ctApi_AstNode_clone(Node** sret, Node** thisArg)
    {
        *sret = Nest_cloneNode(*thisArg);
    }
    int ctApi_AstNode_nodeKind(Node** thisArg)
    {
        return (*thisArg)->nodeKind;
    }
    void ctApi_AstNode_nodeKindName(StringRef* sret, Node** thisArg)
    {
        *sret = fromCStr(Nest_nodeKindName(*thisArg));
    }
    void ctApi_AstNode_toString(StringRef* sret, Node** thisArg)
    {
        *sret = fromCStr(Nest_toString(*thisArg));
    }
    void ctApi_AstNode_toStringExt(StringRef* sret, Node** thisArg)
    {
        *sret = fromCStr(Nest_toString(*thisArg));
    }
    void ctApi_AstNode_location(Location* sret, Node** thisArg)
    {
        *sret = (*thisArg)->location;
    }
    void ctApi_AstNode_children(Node** thisArg, Node*** retBegin, Node*** retEnd)
    {
        NodeRange r = Nest_nodeChildren(*thisArg);
        *retBegin = r.beginPtr;
        *retEnd = r.endPtr;
    }
    void ctApi_AstNode_getChild(Node** sret, Node** thisArg, int n)
    {
        NodeRange r = Nest_nodeChildren(*thisArg);
        *sret = r.beginPtr[n];
    }
    void ctApi_AstNode_referredNodes(Node** thisArg, Node*** retBegin, Node*** retEnd)
    {
        NodeRange r = Nest_nodeReferredNodes(*thisArg);
        *retBegin = r.beginPtr;
        *retEnd = r.endPtr;
    }
    bool ctApi_AstNode_hasProperty(Node** thisArg, StringRef name)
    {
        return Nest_hasProperty(*thisArg, name.begin);
    }
    bool ctApi_AstNode_getPropertyString(Node** thisArg, StringRef name, StringRef* value)
    {
        const StringRef* res = Nest_getPropertyString(*thisArg, name.begin);
        if ( res ) {
            *value = *res;
        }
        return res != nullptr;
    }
    bool ctApi_AstNode_getPropertyInt(Node** thisArg, StringRef name, int* value)
    {
        const int* res = Nest_getPropertyInt(*thisArg, name.begin);
        if ( res )
            *value = *res;
        return res != nullptr;
    }
    bool ctApi_AstNode_getPropertyNode(Node** thisArg, StringRef name, Node** value)
    {
        Node*const* res = Nest_getPropertyNode(*thisArg, name.begin);
        if ( res )
            *value = *res;
        return res != nullptr;
    }
    bool ctApi_AstNode_getPropertyType(Node** thisArg, StringRef name, TypeRef* value)
    {
        const TypeRef* res = Nest_getPropertyType(*thisArg, name.begin);
        if ( res )
            *value = *res;
        return res != nullptr;
    }
    void ctApi_AstNode_setPropertyString(Node** thisArg, StringRef name, StringRef value)
    {
        Nest_setPropertyString(*thisArg, name.begin, value);
    }
    void ctApi_AstNode_setPropertyInt(Node** thisArg, StringRef name, int value)
    {
        Nest_setPropertyInt(*thisArg, name.begin, value);
    }
    void ctApi_AstNode_setPropertyNode(Node** thisArg, StringRef name, Node* value)
    {
        Nest_setPropertyNode(*thisArg, name.begin, value);
    }
    void ctApi_AstNode_setPropertyType(Node** thisArg, StringRef name, TypeRef value)
    {
        Nest_setPropertyType(*thisArg, name.begin, value);
    }
    void ctApi_AstNode_setContext(Node** thisArg, CompilationContext* context)
    {
        Nest_setContext(*thisArg, context);
    }
    void ctApi_AstNode_computeType(Node** thisArg)
    {
        Nest_computeType(*thisArg);
    }
    void ctApi_AstNode_semanticCheck(Node** thisArg)
    {
        Nest_semanticCheck(*thisArg);
    }
    void ctApi_AstNode_clearCompilationState(Node** thisArg)
    {
        Nest_clearCompilationState(*thisArg);
    }
    void ctApi_AstNode_context(CompilationContext** sret, Node** thisArg)
    {
        *sret = (*thisArg)->context;
    }
    bool ctApi_AstNode_hasError(Node** thisArg)
    {
        return (*thisArg)->nodeError != 0;
    }
    bool ctApi_AstNode_isSemanticallyChecked(Node** thisArg)
    {
        return (*thisArg)->nodeSemanticallyChecked != 0;
    }
    void ctApi_AstNode_type(TypeRef* sret, Node** thisArg)
    {
        *sret = (*thisArg)->type;
    }
    bool ctApi_AstNode_isExplained(Node** thisArg)
    {
        return (*thisArg)->explanation != NULL;
    }
    void ctApi_AstNode_explanation(Node** sret, Node** thisArg)
    {
        *sret = Nest_explanation(*thisArg);
    }
    void ctApi_AstNode_curExplanation(Node** sret, Node** thisArg)
    {
        *sret = (*thisArg)->explanation;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Feather nodes creation functions
    //
    void ctApi_Feather_mkNodeList(Node** sret, Location* loc, Node** childrenBegin, Node** childrenEnd, bool voidResult)
    {
        NodeRange r = { childrenBegin, childrenEnd };
        if ( voidResult )
            *sret = Feather_mkNodeListVoid(*loc, r);
        else
            *sret = Feather_mkNodeList(*loc, r);
    }
    void ctApi_Feather_addToNodeList(Node** sret, Node* prevList, Node* element)
    {
        *sret =  Feather_addToNodeList(prevList, element);
    }
    void ctApi_Feather_appendNodeList(Node** sret, Node* list, Node* newNodes)
    {
        *sret = Feather_appendNodeList(list, newNodes);
    }

    void ctApi_Feather_mkNop(Node** sret, Location* loc)
    {
        *sret = Feather_mkNop(*loc);
    }
    void ctApi_Feather_mkTypeNode(Node** sret, Location* loc, TypeRef type)
    {
        *sret = Feather_mkTypeNode(*loc, type);
    }
    void ctApi_Feather_mkBackendCode(Node** sret, Location* loc, StringRef code, int evalMode)
    {
        *sret = Feather_mkBackendCode(*loc, code, (EvalMode) evalMode);
    }
    void ctApi_Feather_mkLocalSpace(Node** sret, Location* loc, Node** childrenBegin, Node** childrenEnd)
    {
        NodeRange r = { childrenBegin, childrenEnd };
        *sret = Feather_mkLocalSpace(*loc, r);
    }
    void ctApi_Feather_mkGlobalConstructAction(Node** sret, Location* loc, Node* action)
    {
        *sret = Feather_mkGlobalConstructAction(*loc, action);
    }
    void ctApi_Feather_mkGlobalDestructAction(Node** sret, Location* loc, Node* action)
    {
        *sret = Feather_mkGlobalDestructAction(*loc, action);
    }
    void ctApi_Feather_mkScopeDestructAction(Node** sret, Location* loc, Node* action)
    {
        *sret = Feather_mkScopeDestructAction(*loc, action);
    }
    void ctApi_Feather_mkTempDestructAction(Node** sret, Location* loc, Node* action)
    {
        *sret = Feather_mkTempDestructAction(*loc, action);
    }

    void ctApi_Feather_mkFunction(Node** sret, Location* loc, StringRef name, Node* resType, Node** paramsBegin, Node** paramsEnd, Node* body, int evalMode)
    {
        NodeRange r = { paramsBegin, paramsEnd };
        *sret = Feather_mkFunction(*loc, name, resType, r, body);
    }
    void ctApi_Feather_mkClass(Node** sret, Location* loc, StringRef name, Node** fieldsBegin, Node** fieldsEnd, int evalMode)
    {
        NodeRange r = { fieldsBegin, fieldsEnd };
        *sret = Feather_mkClass(*loc, name, r);
    }
    void ctApi_Feather_mkVar(Node** sret, Location* loc, StringRef name, Node* type, int evalMode)
    {
        *sret = Feather_mkVar(*loc, name, type);
        Feather_setEvalMode(*sret, (EvalMode) evalMode);
    }

    void ctApi_Feather_mkCtValue(Node** sret, Location* loc, TypeRef type, StringRef data)
    {
        *sret = Feather_mkCtValue(*loc, type, data);
    }
    void ctApi_Feather_mkNull(Node** sret, Location* loc, Node* typeNode)
    {
        *sret = Feather_mkNull(*loc, typeNode);
    }
    void ctApi_Feather_mkVarRef(Node** sret, Location* loc, Node* varDecl)
    {
        *sret = Feather_mkVarRef(*loc, varDecl);
    }
    void ctApi_Feather_mkFieldRef(Node** sret, Location* loc, Node* obj, Node* fieldDecl)
    {
        *sret = Feather_mkFieldRef(*loc, obj, fieldDecl);
    }
    void ctApi_Feather_mkFunRef(Node** sret, Location* loc, Node* funDecl, Node* resType)
    {
        *sret = Feather_mkFunRef(*loc, funDecl, resType);
    }
    void ctApi_Feather_mkFunCall(Node** sret, Location* loc, Node* funDecl, Node** argsBegin, Node** argsEnd)
    {
        NodeRange r = { argsBegin, argsEnd };
        *sret = Feather_mkFunCall(*loc, funDecl, r);
    }
    void ctApi_Feather_mkMemLoad(Node** sret, Location* loc, Node* exp)
    {
        *sret = Feather_mkMemLoad(*loc, exp);
    }
    void ctApi_Feather_mkMemStore(Node** sret, Location* loc, Node* value, Node* address)
    {
        *sret = Feather_mkMemStore(*loc, value, address);
    }
    void ctApi_Feather_mkBitcast(Node** sret, Location* loc, Node* destType, Node* exp)
    {
        *sret = Feather_mkBitcast(*loc, destType, exp);
    }
    void ctApi_Feather_mkConditional(Node** sret, Location* loc, Node* condition, Node* alt1, Node* alt2)
    {
        *sret = Feather_mkConditional(*loc, condition, alt1, alt2);
    }

    void ctApi_Feather_mkIf(Node** sret, Location* loc, Node* condition, Node* thenClause, Node* elseClause, bool isCt)
    {
        *sret = Feather_mkIf(*loc, condition, thenClause, elseClause);
        if ( isCt )
            Feather_setEvalMode(*sret, modeCt);
    }
    void ctApi_Feather_mkWhile(Node** sret, Location* loc, Node* condition, Node* body, Node* step, bool isCt)
    {
        *sret = Feather_mkWhile(*loc, condition, body, step);
        if ( isCt )
            Feather_setEvalMode(*sret, modeCt);
    }
    void ctApi_Feather_mkBreak(Node** sret, Location* loc)
    {
        *sret = Feather_mkBreak(*loc);
    }
    void ctApi_Feather_mkContinue(Node** sret, Location* loc)
    {
        *sret = Feather_mkContinue(*loc);
    }
    void ctApi_Feather_mkReturn(Node** sret, Location* loc, Node* exp)
    {
        *sret = Feather_mkReturn(*loc, exp);
    }
}

void Feather_registerCtApiFunctions(Backend* backend)
{
    backend->ctApiRegisterFun(backend, "$meta.SourceCode.fromFilename",       (void*) &ctApi_SourceCode_fromFilename);
    backend->ctApiRegisterFun(backend, "$meta.SourceCode.filename",           (void*) &ctApi_SourceCode_filename);

    backend->ctApiRegisterFun(backend, "$meta.Location.getCorrespondingCode", (void*) &ctApi_Location_getCorrespondingCode);

    backend->ctApiRegisterFun(backend, "$meta.report",                        (void*) &ctApi_report);
    backend->ctApiRegisterFun(backend, "$meta.raise",                         (void*) &ctApi_raise);

    backend->ctApiRegisterFun(backend, "$meta.CompilationContext.evalMode",   (void*) &ctApi_CompilationContext_evalMode);
    backend->ctApiRegisterFun(backend, "$meta.CompilationContext.sourceCode", (void*) &ctApi_CompilationContext_sourceCode);

    backend->ctApiRegisterFun(backend, "$meta.AstType.typeKind",              (void*) &ctApi_AstType_typeKind);
    backend->ctApiRegisterFun(backend, "$meta.AstType.toString",              (void*) &ctApi_AstType_toString);
    backend->ctApiRegisterFun(backend, "$meta.AstType.hasStorage",            (void*) &ctApi_AstType_hasStorage);
    backend->ctApiRegisterFun(backend, "$meta.AstType.numReferences",         (void*) &ctApi_AstType_numReferences);
    backend->ctApiRegisterFun(backend, "$meta.AstType.mode",                  (void*) &ctApi_AstType_mode);
    backend->ctApiRegisterFun(backend, "$meta.AstType.canBeUsedAtCt",         (void*) &ctApi_AstType_canBeUsedAtCt);
    backend->ctApiRegisterFun(backend, "$meta.AstType.canBeUsedAtRt",         (void*) &ctApi_AstType_canBeUsedAtRt);

    backend->ctApiRegisterFun(backend, "$meta.AstNode.clone",                 (void*) &ctApi_AstNode_clone);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.nodeKind",              (void*) &ctApi_AstNode_nodeKind);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.nodeKindName",          (void*) &ctApi_AstNode_nodeKindName);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.toString",              (void*) &ctApi_AstNode_toString);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.toStringExt",           (void*) &ctApi_AstNode_toStringExt);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.location",              (void*) &ctApi_AstNode_location);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.children",              (void*) &ctApi_AstNode_children);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.getChild",              (void*) &ctApi_AstNode_getChild);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.referredNodes",         (void*) &ctApi_AstNode_referredNodes);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.hasProperty",           (void*) &ctApi_AstNode_hasProperty);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.getPropertyString",     (void*) &ctApi_AstNode_getPropertyString);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.getPropertyInt",        (void*) &ctApi_AstNode_getPropertyInt);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.getPropertyNode",       (void*) &ctApi_AstNode_getPropertyNode);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.getPropertyType",       (void*) &ctApi_AstNode_getPropertyType);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.setPropertyString",     (void*) &ctApi_AstNode_setPropertyString);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.setPropertyInt",        (void*) &ctApi_AstNode_setPropertyInt);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.setPropertyNode",       (void*) &ctApi_AstNode_setPropertyNode);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.setPropertyType",       (void*) &ctApi_AstNode_setPropertyType);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.setContext",            (void*) &ctApi_AstNode_setContext);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.computeType",           (void*) &ctApi_AstNode_computeType);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.semanticCheck",         (void*) &ctApi_AstNode_semanticCheck);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.clearCompilationState", (void*) &ctApi_AstNode_clearCompilationState);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.context",               (void*) &ctApi_AstNode_context);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.hasError",              (void*) &ctApi_AstNode_hasError);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.isSemanticallyChecked", (void*) &ctApi_AstNode_isSemanticallyChecked);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.type",                  (void*) &ctApi_AstNode_type);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.isExplained",           (void*) &ctApi_AstNode_isExplained);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.explanation",           (void*) &ctApi_AstNode_explanation);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.curExplanation",        (void*) &ctApi_AstNode_curExplanation);

    backend->ctApiRegisterFun(backend, "$meta.Feather.mkNodeList",                (void*) &ctApi_Feather_mkNodeList);
    backend->ctApiRegisterFun(backend, "$meta.Feather.addToNodeList",             (void*) &ctApi_Feather_addToNodeList);
    backend->ctApiRegisterFun(backend, "$meta.Feather.appendNodeList",            (void*) &ctApi_Feather_appendNodeList);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkNop",                     (void*) &ctApi_Feather_mkNop);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkTypeNode",                (void*) &ctApi_Feather_mkTypeNode);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkBackendCode",             (void*) &ctApi_Feather_mkBackendCode);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkLocalSpace",              (void*) &ctApi_Feather_mkLocalSpace);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkGlobalConstructAction",   (void*) &ctApi_Feather_mkGlobalConstructAction);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkGlobalDestructAction",    (void*) &ctApi_Feather_mkGlobalDestructAction);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkScopeDestructAction",     (void*) &ctApi_Feather_mkScopeDestructAction);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkTempDestructAction",      (void*) &ctApi_Feather_mkTempDestructAction);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkFunction",                (void*) &ctApi_Feather_mkFunction);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkClass",                   (void*) &ctApi_Feather_mkClass);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkVar",                     (void*) &ctApi_Feather_mkVar);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkCtValue",                 (void*) &ctApi_Feather_mkCtValue);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkNull",                    (void*) &ctApi_Feather_mkNull);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkVarRef",                  (void*) &ctApi_Feather_mkVarRef);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkFieldRef",                (void*) &ctApi_Feather_mkFieldRef);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkFunRef",                  (void*) &ctApi_Feather_mkFunRef);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkFunCall",                 (void*) &ctApi_Feather_mkFunCall);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkMemLoad",                 (void*) &ctApi_Feather_mkMemLoad);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkMemStore",                (void*) &ctApi_Feather_mkMemStore);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkBitcast",                 (void*) &ctApi_Feather_mkBitcast);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkConditional",             (void*) &ctApi_Feather_mkConditional);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkIf",                      (void*) &ctApi_Feather_mkIf);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkWhile",                   (void*) &ctApi_Feather_mkWhile);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkBreak",                   (void*) &ctApi_Feather_mkBreak);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkContinue",                (void*) &ctApi_Feather_mkContinue);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkReturn",                  (void*) &ctApi_Feather_mkReturn);
}