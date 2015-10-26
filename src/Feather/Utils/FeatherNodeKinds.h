#pragma once

// The IDs for all the feather node kinds
extern int nkFeatherNop;
extern int nkFeatherTypeNode;
extern int nkFeatherBackendCode;
extern int nkFeatherNodeList;
extern int nkFeatherLocalSpace;
extern int nkFeatherGlobalConstructAction;
extern int nkFeatherGlobalDestructAction;
extern int nkFeatherScopeDestructAction;
extern int nkFeatherTempDestructAction;
extern int nkFeatherChangeMode;

extern int nkFeatherDeclFunction;
extern int nkFeatherDeclClass;
extern int nkFeatherDeclVar;

extern int nkFeatherExpCtValue;
extern int nkFeatherExpNull;
extern int nkFeatherExpVarRef;
extern int nkFeatherExpFieldRef;
extern int nkFeatherExpFunRef;
extern int nkFeatherExpFunCall;
extern int nkFeatherExpMemLoad;
extern int nkFeatherExpMemStore;
extern int nkFeatherExpBitcast;
extern int nkFeatherExpConditional;

extern int nkFeatherStmtIf;
extern int nkFeatherStmtWhile;
extern int nkFeatherStmtBreak;
extern int nkFeatherStmtContinue;
extern int nkFeatherStmtReturn;

