#pragma once

namespace Feather
{
    enum FeatherNodeKinds
    {
        nkFeatherNop = 1,
        nkFeatherTypeNode,
        nkFeatherBackendCode,
        nkFeatherNodeList,
        nkFeatherLocalSpace,
        nkFeatherGlobalConstructAction,
        nkFeatherGlobalDestructAction,
        nkFeatherScopeDestructAction,
        nkFeatherTempDestructAction,
        
        nkFeatherDeclFunction,
        nkFeatherDeclClass,
        nkFeatherDeclVar,
        
        nkFeatherExpCtValue,
        nkFeatherExpNull,
        nkFeatherExpStackAlloc,
        nkFeatherExpVarRef,
        nkFeatherExpFieldRef,
        nkFeatherExpFunRef,
        nkFeatherExpFunCall,
        nkFeatherExpMemLoad,
        nkFeatherExpMemStore,
        nkFeatherExpBitcast,
        nkFeatherExpConditional,
        nkFeatherExpChangeMode,
        
        nkFeatherStmtIf,
        nkFeatherStmtWhile,
        nkFeatherStmtBreak,
        nkFeatherStmtContinue,
        nkFeatherStmtReturn,
        
        nextNodeKindValue,
    };
}
