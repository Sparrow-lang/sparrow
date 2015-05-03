#include <StdInc.h>
#include "CtApiFunctions.h"

#include <Nodes/FeatherNodes.h>
#include <Util/StringData.h>
#include <Util/Context.h>

#include <Nest/Backend/Backend.h>
#include <Nest/Frontend/SourceCode.h>

using namespace Feather;
using namespace Nest;

namespace
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Diagnostic
    //
    void ctApi_SourceCode_fromFilename(const SourceCode** sret, StringData filename)
    {
        *sret = theCompiler().getSourceCodeForFilename(filename.toStdString());
    }
    void ctApi_SourceCode_filename(StringData* sret, SourceCode** thisArg)
    {
        *sret = StringData(*new string((*thisArg)->filename()));
    }

    void ctApi_Location_getCorrespondingCode(StringData* sret, Location* thisArg)
    {

        *sret = StringData(*new string(thisArg->getCorrespondingCode()));
    }

    void ctApi_report(int type, StringData message, Location* location)
    {
        Location loc = location ? *location : Location();
        Nest::Common::diagnosticReporter().report((Nest::Common::DiagnosticSeverity) type, message.toStdString(), loc, true);
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
        return (*thisArg)->evalMode();
    }

    void ctApi_CompilationContext_sourceCode(SourceCode** sret, CompilationContext** thisArg)
    {
        *sret = (*thisArg)->sourceCode();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // AstType
    //
    int ctApi_AstType_typeId(Type** thisArg)
    {
        return (*thisArg)->typeId();
    }
    void ctApi_AstType_toString(StringData* sret, Type** thisArg)
    {
        *sret = StringData(*new string((*thisArg)->toString()));
    }
    bool ctApi_AstType_hasStorage(Type** thisArg)
    {
        return (*thisArg)->hasStorage();
    }
    int ctApi_AstType_numReferences(Type** thisArg)
    {
        return (*thisArg)->noReferences();
    }
    int ctApi_AstType_mode(Type** thisArg)
    {
        return (*thisArg)->mode();
    }
    bool ctApi_AstType_canBeUsedAtCt(Type** thisArg)
    {
        return (*thisArg)->canBeUsedAtCt();
    }
    bool ctApi_AstType_canBeUsedAtRt(Type** thisArg)
    {
        return (*thisArg)->canBeUsedAtRt();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // AstNode
    //
    void ctApi_AstNode_clone(Node** sret, Node** thisArg)
    {
        *sret = (*thisArg)->clone();
    }
    int ctApi_AstNode_nodeKind(Node** thisArg)
    {
        return (*thisArg)->nodeKind();
    }
    void ctApi_AstNode_nodeKindName(StringData* sret, Node** thisArg)
    {
        *sret = StringData(*new string((*thisArg)->nodeKindName()));
    }
    void ctApi_AstNode_toString(StringData* sret, Node** thisArg)
    {
        *sret = StringData(*new string((*thisArg)->toString()));
    }
    void ctApi_AstNode_toStringExt(StringData* sret, Node** thisArg)
    {
        ostringstream oss;
        oss << *thisArg;
        *sret = StringData(*new string(oss.str()));
    }
    void ctApi_AstNode_location(Location* sret, Node** thisArg)
    {
        *sret = (*thisArg)->location();
    }
    void ctApi_AstNode_children(Node** thisArg, Node*** retBegin, Node*** retEnd)
    {
        *retBegin = &*(*thisArg)->children().begin();
        *retEnd = &*(*thisArg)->children().end();
    }
    void ctApi_AstNode_getChild(Node** sret, Node** thisArg, int n)
    {
        *sret = (*thisArg)->children()[n];
    }
    void ctApi_AstNode_referredNodes(Node** thisArg, Node*** retBegin, Node*** retEnd)
    {
        *retBegin = &*(*thisArg)->referredNodes().begin();
        *retEnd = &*(*thisArg)->referredNodes().end();
    }
    bool ctApi_AstNode_hasProperty(Node** thisArg, StringData name)
    {
        return (*thisArg)->hasProperty(name.begin);
    }
    bool ctApi_AstNode_getPropertyString(Node** thisArg, StringData name, StringData* value)
    {
        const string* res = (*thisArg)->getPropertyString(name.begin);
        if ( res )
            *value = StringData(*res);
        return res != nullptr;
    }
    bool ctApi_AstNode_getPropertyInt(Node** thisArg, StringData name, int* value)
    {
        const int* res = (*thisArg)->getPropertyInt(name.begin);
        if ( res )
            *value = *res;
        return res != nullptr;
    }
    bool ctApi_AstNode_getPropertyNode(Node** thisArg, StringData name, Node** value)
    {
        Node*const* res = (*thisArg)->getPropertyNode(name.begin);
        if ( res )
            *value = *res;
        return res != nullptr;
    }
    bool ctApi_AstNode_getPropertyType(Node** thisArg, StringData name, Type** value)
    {
        Type*const* res = (*thisArg)->getPropertyType(name.begin);
        if ( res )
            *value = *res;
        return res != nullptr;
    }
    void ctApi_AstNode_setPropertyString(Node** thisArg, StringData name, StringData value)
    {
        (*thisArg)->setProperty(name.begin, value.toStdString());
    }
    void ctApi_AstNode_setPropertyInt(Node** thisArg, StringData name, int value)
    {
        (*thisArg)->setProperty(name.begin, value);
    }
    void ctApi_AstNode_setPropertyNode(Node** thisArg, StringData name, Node* value)
    {
        (*thisArg)->setProperty(name.begin, value);
    }
    void ctApi_AstNode_setPropertyType(Node** thisArg, StringData name, Type* value)
    {
        (*thisArg)->setProperty(name.begin, value);
    }
    void ctApi_AstNode_setContext(Node** thisArg, CompilationContext* context)
    {
        (*thisArg)->setContext(context);
    }
    void ctApi_AstNode_computeType(Node** thisArg)
    {
        (*thisArg)->computeType();
    }
    void ctApi_AstNode_semanticCheck(Node** thisArg)
    {
        (*thisArg)->semanticCheck();
    }
    void ctApi_AstNode_clearCompilationState(Node** thisArg)
    {
        (*thisArg)->clearCompilationState();
    }
    void ctApi_AstNode_context(CompilationContext** sret, Node** thisArg)
    {
        *sret = (*thisArg)->context();
    }
    bool ctApi_AstNode_hasError(Node** thisArg)
    {
        return (*thisArg)->hasError();
    }
    bool ctApi_AstNode_isSemanticallyChecked(Node** thisArg)
    {
        return (*thisArg)->isSemanticallyChecked();
    }
    void ctApi_AstNode_type(Type** sret, Node** thisArg)
    {
        *sret = (*thisArg)->type();
    }
    bool ctApi_AstNode_isExplained(Node** thisArg)
    {
        return (*thisArg)->isExplained();
    }
    void ctApi_AstNode_explanation(Node** sret, Node** thisArg)
    {
        *sret = (*thisArg)->explanation();
    }
    void ctApi_AstNode_curExplanation(Node** sret, Node** thisArg)
    {
        *sret = (*thisArg)->curExplanation();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Feather nodes creation functions
    //
    void ctApi_Feather_mkNodeList(NodeList** sret, Location* loc, Node** childrenBegin, Node** childrenEnd, bool voidResult)
    {
        *sret = mkNodeList(*loc, NodeVector(childrenBegin, childrenEnd), voidResult);
    }
    void ctApi_Feather_addToNodeList(NodeList** sret, NodeList* prevList, Node* element)
    {
        *sret =  addToNodeList(prevList, element);
    }
    void ctApi_Feather_appendNodeList(NodeList** sret, NodeList* list, NodeList* newNodes)
    {
        *sret = appendNodeList(list, newNodes);
    }

    void ctApi_Feather_mkNop(Node** sret, Location* loc)
    {
        *sret = mkNop(*loc);
    }
    void ctApi_Feather_mkTypeNode(Node** sret, Location* loc, Type* type)
    {
        *sret = mkTypeNode(*loc, type);
    }
    void ctApi_Feather_mkBackendCode(Node** sret, Location* loc, StringData code, int evalMode)
    {
        *sret = mkBackendCode(*loc, code.toStdString(), (EvalMode) evalMode);
    }
    void ctApi_Feather_mkLocalSpace(Node** sret, Location* loc, Node** childrenBegin, Node** childrenEnd)
    {
        *sret = mkLocalSpace(*loc, NodeVector(childrenBegin, childrenEnd));
    }
    void ctApi_Feather_mkGlobalConstructAction(Node** sret, Location* loc, Node* action)
    {
        *sret = mkGlobalConstructAction(*loc, action);
    }
    void ctApi_Feather_mkGlobalDestructAction(Node** sret, Location* loc, Node* action)
    {
        *sret = mkGlobalDestructAction(*loc, action);
    }
    void ctApi_Feather_mkScopeDestructAction(Node** sret, Location* loc, Node* action)
    {
        *sret = mkScopeDestructAction(*loc, action);
    }
    void ctApi_Feather_mkTempDestructAction(Node** sret, Location* loc, Node* action)
    {
        *sret = mkTempDestructAction(*loc, action);
    }

    void ctApi_Feather_mkFunction(Node** sret, Location* loc, StringData name, Node* resType, Node** paramsBegin, Node** paramsEnd, Node* body, int evalMode)
    {
        *sret = mkFunction(*loc, name.toStdString(), resType, NodeVector(paramsBegin, paramsEnd), body);
    }
    void ctApi_Feather_mkClass(Node** sret, Location* loc, StringData name, Node** fieldsBegin, Node** fieldsEnd, int evalMode)
    {
        *sret = mkClass(*loc, name.toStdString(), NodeVector(fieldsBegin, fieldsEnd));
    }
    void ctApi_Feather_mkVar(Node** sret, Location* loc, StringData name, Node* type, int evalMode)
    {
        *sret = mkVar(*loc, name.toStdString(), type, 0, (EvalMode) evalMode);
    }

    void ctApi_Feather_mkCtValue(Node** sret, Location* loc, Type* type, StringData data)
    {
        *sret = mkCtValue(*loc, type, data.toStdString());
    }
    void ctApi_Feather_mkNull(Node** sret, Location* loc, Node* typeNode)
    {
        *sret = mkNull(*loc, typeNode);
    }
    void ctApi_Feather_mkStackAlloc(Node** sret, Location* loc, Node* typeNode, int numElements)
    {
        *sret = mkStackAlloc(*loc, typeNode, numElements);
    }
    void ctApi_Feather_mkVarRef(Node** sret, Location* loc, Node* varDecl)
    {
        *sret = mkVarRef(*loc, varDecl);
    }
    void ctApi_Feather_mkFieldRef(Node** sret, Location* loc, Node* obj, Node* fieldDecl)
    {
        *sret = mkFieldRef(*loc, obj, fieldDecl);
    }
    void ctApi_Feather_mkFunRef(Node** sret, Location* loc, Node* funDecl, Node* resType)
    {
        *sret = mkFunRef(*loc, funDecl, resType);
    }
    void ctApi_Feather_mkFunCall(Node** sret, Location* loc, Node* funDecl, Node** argsBegin, Node** argsEnd)
    {
        *sret = mkFunCall(*loc, funDecl, NodeVector(argsBegin, argsEnd));
    }
    void ctApi_Feather_mkMemLoad(Node** sret, Location* loc, Node* exp)
    {
        *sret = mkMemLoad(*loc, exp);
    }
    void ctApi_Feather_mkMemStore(Node** sret, Location* loc, Node* value, Node* address)
    {
        *sret = mkMemStore(*loc, value, address);
    }
    void ctApi_Feather_mkBitcast(Node** sret, Location* loc, Node* destType, Node* exp)
    {
        *sret = mkBitcast(*loc, destType, exp);
    }
    void ctApi_Feather_mkConditional(Node** sret, Location* loc, Node* condition, Node* alt1, Node* alt2)
    {
        *sret = mkConditional(*loc, condition, alt1, alt2);
    }

    void ctApi_Feather_mkIf(Node** sret, Location* loc, Node* condition, Node* thenClause, Node* elseClause, bool isCt)
    {
        *sret = mkIf(*loc, condition, thenClause, elseClause, isCt);
    }
    void ctApi_Feather_mkWhile(Node** sret, Location* loc, Node* condition, Node* body, Node* step, bool isCt)
    {
        *sret = mkWhile(*loc, condition, body, step, isCt);
    }
    void ctApi_Feather_mkBreak(Node** sret, Location* loc)
    {
        *sret = mkBreak(*loc);
    }
    void ctApi_Feather_mkContinue(Node** sret, Location* loc)
    {
        *sret = mkContinue(*loc);
    }
    void ctApi_Feather_mkReturn(Node** sret, Location* loc, Node* exp)
    {
        *sret = mkReturn(*loc, exp);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Util
    //
    void ctApi_Util_getParentDecl(Node** sret, CompilationContext* context)
    {
        *sret = getParentDecl(context);
    }
    void ctApi_Util_getParentFun(Function** sret, CompilationContext* context)
    {
        *sret = getParentFun(context);
    }
    void ctApi_Util_getParentClass(Class** sret, CompilationContext* context)
    {
        *sret = getParentClass(context);
    }
    void ctApi_Util_getParentLoop(Node** sret, CompilationContext* context)
    {
        *sret = getParentLoop(context);
    }
    void ctApi_Util_getSymTabContext(CompilationContext** sret, CompilationContext* context)
    {
        *sret = getSymTabContext(context);
    }
}

void Feather::registerCtApiFunctions(Backend& backend)
{
    backend.ctApiRegisterFun("$Meta.SourceCode.fromFilename",       (void*) &ctApi_SourceCode_fromFilename);
    backend.ctApiRegisterFun("$Meta.SourceCode.filename",           (void*) &ctApi_SourceCode_filename);

    backend.ctApiRegisterFun("$Meta.Location.getCorrespondingCode", (void*) &ctApi_Location_getCorrespondingCode);

    backend.ctApiRegisterFun("$Meta.report",                        (void*) &ctApi_report);
    backend.ctApiRegisterFun("$Meta.raise",                         (void*) &ctApi_raise);

    backend.ctApiRegisterFun("$Meta.CompilationContext.evalMode",   (void*) &ctApi_CompilationContext_evalMode);
    backend.ctApiRegisterFun("$Meta.CompilationContext.sourceCode", (void*) &ctApi_CompilationContext_sourceCode);

    backend.ctApiRegisterFun("$Meta.AstType.typeId",                (void*) &ctApi_AstType_typeId);
    backend.ctApiRegisterFun("$Meta.AstType.toString",              (void*) &ctApi_AstType_toString);
    backend.ctApiRegisterFun("$Meta.AstType.hasStorage",            (void*) &ctApi_AstType_hasStorage);
    backend.ctApiRegisterFun("$Meta.AstType.numReferences",         (void*) &ctApi_AstType_numReferences);
    backend.ctApiRegisterFun("$Meta.AstType.mode",                  (void*) &ctApi_AstType_mode);
    backend.ctApiRegisterFun("$Meta.AstType.canBeUsedAtCt",         (void*) &ctApi_AstType_canBeUsedAtCt);
    backend.ctApiRegisterFun("$Meta.AstType.canBeUsedAtRt",         (void*) &ctApi_AstType_canBeUsedAtRt);

    backend.ctApiRegisterFun("$Meta.AstNode.clone",                 (void*) &ctApi_AstNode_clone);
    backend.ctApiRegisterFun("$Meta.AstNode.nodeKind",              (void*) &ctApi_AstNode_nodeKind);
    backend.ctApiRegisterFun("$Meta.AstNode.nodeKindName",          (void*) &ctApi_AstNode_nodeKindName);
    backend.ctApiRegisterFun("$Meta.AstNode.toString",              (void*) &ctApi_AstNode_toString);
    backend.ctApiRegisterFun("$Meta.AstNode.toStringExt",           (void*) &ctApi_AstNode_toStringExt);
    backend.ctApiRegisterFun("$Meta.AstNode.location",              (void*) &ctApi_AstNode_location);
    backend.ctApiRegisterFun("$Meta.AstNode.children",              (void*) &ctApi_AstNode_children);
    backend.ctApiRegisterFun("$Meta.AstNode.getChild",              (void*) &ctApi_AstNode_getChild);
    backend.ctApiRegisterFun("$Meta.AstNode.referredNodes",         (void*) &ctApi_AstNode_referredNodes);
    backend.ctApiRegisterFun("$Meta.AstNode.hasProperty",           (void*) &ctApi_AstNode_hasProperty);
    backend.ctApiRegisterFun("$Meta.AstNode.getPropertyString",     (void*) &ctApi_AstNode_getPropertyString);
    backend.ctApiRegisterFun("$Meta.AstNode.getPropertyInt",        (void*) &ctApi_AstNode_getPropertyInt);
    backend.ctApiRegisterFun("$Meta.AstNode.getPropertyNode",       (void*) &ctApi_AstNode_getPropertyNode);
    backend.ctApiRegisterFun("$Meta.AstNode.getPropertyType",       (void*) &ctApi_AstNode_getPropertyType);
    backend.ctApiRegisterFun("$Meta.AstNode.setPropertyString",     (void*) &ctApi_AstNode_setPropertyString);
    backend.ctApiRegisterFun("$Meta.AstNode.setPropertyInt",        (void*) &ctApi_AstNode_setPropertyInt);
    backend.ctApiRegisterFun("$Meta.AstNode.setPropertyNode",       (void*) &ctApi_AstNode_setPropertyNode);
    backend.ctApiRegisterFun("$Meta.AstNode.setPropertyType",       (void*) &ctApi_AstNode_setPropertyType);
    backend.ctApiRegisterFun("$Meta.AstNode.setContext",            (void*) &ctApi_AstNode_setContext);
    backend.ctApiRegisterFun("$Meta.AstNode.computeType",           (void*) &ctApi_AstNode_computeType);
    backend.ctApiRegisterFun("$Meta.AstNode.semanticCheck",         (void*) &ctApi_AstNode_semanticCheck);
    backend.ctApiRegisterFun("$Meta.AstNode.clearCompilationState", (void*) &ctApi_AstNode_clearCompilationState);
    backend.ctApiRegisterFun("$Meta.AstNode.context",               (void*) &ctApi_AstNode_context);
    backend.ctApiRegisterFun("$Meta.AstNode.hasError",              (void*) &ctApi_AstNode_hasError);
    backend.ctApiRegisterFun("$Meta.AstNode.isSemanticallyChecked", (void*) &ctApi_AstNode_isSemanticallyChecked);
    backend.ctApiRegisterFun("$Meta.AstNode.type",                  (void*) &ctApi_AstNode_type);
    backend.ctApiRegisterFun("$Meta.AstNode.isExplained",           (void*) &ctApi_AstNode_isExplained);
    backend.ctApiRegisterFun("$Meta.AstNode.explanation",           (void*) &ctApi_AstNode_explanation);
    backend.ctApiRegisterFun("$Meta.AstNode.curExplanation",        (void*) &ctApi_AstNode_curExplanation);

    backend.ctApiRegisterFun("$Meta.Feather.mkNodeList",                (void*) &ctApi_Feather_mkNodeList);
    backend.ctApiRegisterFun("$Meta.Feather.addToNodeList",             (void*) &ctApi_Feather_addToNodeList);
    backend.ctApiRegisterFun("$Meta.Feather.appendNodeList",            (void*) &ctApi_Feather_appendNodeList);
    backend.ctApiRegisterFun("$Meta.Feather.mkNop",                     (void*) &ctApi_Feather_mkNop);
    backend.ctApiRegisterFun("$Meta.Feather.mkTypeNode",                (void*) &ctApi_Feather_mkTypeNode);
    backend.ctApiRegisterFun("$Meta.Feather.mkBackendCode",             (void*) &ctApi_Feather_mkBackendCode);
    backend.ctApiRegisterFun("$Meta.Feather.mkLocalSpace",              (void*) &ctApi_Feather_mkLocalSpace);
    backend.ctApiRegisterFun("$Meta.Feather.mkGlobalConstructAction",   (void*) &ctApi_Feather_mkGlobalConstructAction);
    backend.ctApiRegisterFun("$Meta.Feather.mkGlobalDestructAction",    (void*) &ctApi_Feather_mkGlobalDestructAction);
    backend.ctApiRegisterFun("$Meta.Feather.mkScopeDestructAction",     (void*) &ctApi_Feather_mkScopeDestructAction);
    backend.ctApiRegisterFun("$Meta.Feather.mkTempDestructAction",      (void*) &ctApi_Feather_mkTempDestructAction);
    backend.ctApiRegisterFun("$Meta.Feather.mkFunction",                (void*) &ctApi_Feather_mkFunction);
    backend.ctApiRegisterFun("$Meta.Feather.mkClass",                   (void*) &ctApi_Feather_mkClass);
    backend.ctApiRegisterFun("$Meta.Feather.mkVar",                     (void*) &ctApi_Feather_mkVar);
    backend.ctApiRegisterFun("$Meta.Feather.mkCtValue",                 (void*) &ctApi_Feather_mkCtValue);
    backend.ctApiRegisterFun("$Meta.Feather.mkNull",                    (void*) &ctApi_Feather_mkNull);
    backend.ctApiRegisterFun("$Meta.Feather.mkStackAlloc",              (void*) &ctApi_Feather_mkStackAlloc);
    backend.ctApiRegisterFun("$Meta.Feather.mkVarRef",                  (void*) &ctApi_Feather_mkVarRef);
    backend.ctApiRegisterFun("$Meta.Feather.mkFieldRef",                (void*) &ctApi_Feather_mkFieldRef);
    backend.ctApiRegisterFun("$Meta.Feather.mkFunRef",                  (void*) &ctApi_Feather_mkFunRef);
    backend.ctApiRegisterFun("$Meta.Feather.mkFunCall",                 (void*) &ctApi_Feather_mkFunCall);
    backend.ctApiRegisterFun("$Meta.Feather.mkMemLoad",                 (void*) &ctApi_Feather_mkMemLoad);
    backend.ctApiRegisterFun("$Meta.Feather.mkMemStore",                (void*) &ctApi_Feather_mkMemStore);
    backend.ctApiRegisterFun("$Meta.Feather.mkBitcast",                 (void*) &ctApi_Feather_mkBitcast);
    backend.ctApiRegisterFun("$Meta.Feather.mkConditional",             (void*) &ctApi_Feather_mkConditional);
    backend.ctApiRegisterFun("$Meta.Feather.mkIf",                      (void*) &ctApi_Feather_mkIf);
    backend.ctApiRegisterFun("$Meta.Feather.mkWhile",                   (void*) &ctApi_Feather_mkWhile);
    backend.ctApiRegisterFun("$Meta.Feather.mkBreak",                   (void*) &ctApi_Feather_mkBreak);
    backend.ctApiRegisterFun("$Meta.Feather.mkContinue",                (void*) &ctApi_Feather_mkContinue);
    backend.ctApiRegisterFun("$Meta.Feather.mkReturn",                  (void*) &ctApi_Feather_mkReturn);

    backend.ctApiRegisterFun("$Meta.Util.getParentDecl",                (void*) &ctApi_Util_getParentDecl);
    backend.ctApiRegisterFun("$Meta.Util.getParentFun",                 (void*) &ctApi_Util_getParentFun);
    backend.ctApiRegisterFun("$Meta.Util.getParentClass",               (void*) &ctApi_Util_getParentClass);
    backend.ctApiRegisterFun("$Meta.Util.getParentLoop",                (void*) &ctApi_Util_getParentLoop);
    backend.ctApiRegisterFun("$Meta.Util.getSymTabContext",             (void*) &ctApi_Util_getSymTabContext);
}
