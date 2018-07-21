#pragma once

#include "Nest/Utils/cppif/NodeUtils.hpp"

FWD_CLASS2(LLVMB, Tr, Scope);
FWD_CLASS2(LLVMB, Tr, TrContext);

namespace LLVMB {
namespace Tr {

using Nest::Node;
using Nest::NodeVector;


/// Represents a Instruction in function body, inside a Scope
///
/// An object of this class adds itself automatically to the Scope on constructor and removes itself
/// from Scope in destructor. Therefore these are intended to be used as scoped objects actions
///
/// This object contains the list of temporary destruct actions that need to be executed (in reverse
/// order) when the instruction finishes.
///
/// \see    Scope, TrContext
class Instruction {
public:
    Instruction(TrContext& context);
    ~Instruction();

    /// Adds an temporary destruct action
    void addTempDestructAction(Node* destructAction);

    /// Translate the destruct actions corresponding to this node
    void outputDestructActions();

    /// Returns the list of destruct actions and removes them from the current instruction
    NodeVector stealDestructActions();

protected:
    Scope& scope_;
    NodeVector destructActions_;
};
} // namespace Tr
} // namespace LLVMB
