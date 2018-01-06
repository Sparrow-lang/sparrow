#pragma once

typedef struct Nest_Node Node;

namespace LLVMB {
namespace Tr {

struct GlobalContext;

//! Makes sure that the given node is translatable.
//!
//! Walks over all the children nodes and make sure they are compiled. If we find nodes that refer
//! to other top-level nodes, make sure these are compiled and prepared for translation.
//!
//! After calling this function, there should be no node that is not semantically checked, which is
//! directly or indirectly needed by the translation of node.
void prepareTranslate(Node* node, GlobalContext& ctx);

} // namespace Tr
} // namespace LLVMB