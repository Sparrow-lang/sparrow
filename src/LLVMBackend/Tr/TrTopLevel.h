#pragma once

typedef struct Nest_Node Node;

namespace llvm {
class Type;
class Module;
} // namespace llvm

namespace LLVMB {
class Module;

namespace Tr {
struct GlobalContext;

//! Main translation method for top-level nodes
void translateTopLevelNode(Node* node, GlobalContext& ctx);

//! Adds the backend code to the current LLVM global context
void translateBackendCode(Node* node, GlobalContext& ctx);

//! Translates the given class
llvm::Type* translateClass(Node* node, GlobalContext& ctx);

//! Translates the given global variable
llvm::Value* translateGlobalVar(Node* node, GlobalContext& ctx);
} // namespace Tr
} // namespace LLVMB
