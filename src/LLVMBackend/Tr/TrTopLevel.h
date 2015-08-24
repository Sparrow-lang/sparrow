#pragma once

FWD_CLASS1(LLVMB, Module)

FWD_CLASS1(llvm, Type);

typedef struct Nest_Node Node;

namespace LLVMB { namespace Tr
{
    // Main translation method: translates a top-level intermediate node into a LLVM node
    void translateTopLevelNode(Node* node, Module& module);

    void translateBackendCode(Node* node, Module& module);
    llvm::Type* translateClass(Node* node, Module& module);
    llvm::Value* translateGlobalVar(Node* node, Module& module);
}}
