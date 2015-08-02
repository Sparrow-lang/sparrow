#pragma once

FWD_CLASS1(LLVMB, Module)
FWD_STRUCT1(Nest, Node);
FWD_CLASS1(Feather, Class);
FWD_CLASS1(Feather, Var);

FWD_CLASS1(llvm, Type);

namespace LLVMB { namespace Tr
{
    // Main translation method: translates a top-level intermediate node into a LLVM node
    void translateTopLevelNode(Nest::Node* node, Module& module);

    void translateBackendCode(Nest::Node* node, Module& module);
    llvm::Type* translateClass(Feather::Class* node, Module& module);
    llvm::Value* translateGlobalVar(Feather::Var* node, Module& module);
}}
