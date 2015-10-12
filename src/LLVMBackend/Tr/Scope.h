#pragma once

#include <Nest/Frontend/Location.h>
#include <Nest/Intermediate/NodeVector.hpp>

FWD_CLASS2(LLVMB,Tr, TrContext);
FWD_CLASS2(LLVMB,Tr, Instruction);

namespace LLVMB { namespace Tr
{
    /// Represents a scope in function body, containing multiple instructions
    /// Usually introduced by a LocalSpace node.
    ///
    /// An object of this class adds itself automatically to the TrContext on constructor and removes itself from
    /// TrContext in destructor. Therefore these are intended to be used as scoped objects
    ///
    /// Beside the list of instructions, a scope has a list of destruct actions. The destruct actions are called in
    /// reverse order at scope end (called from destructor), or when jumping outside the scope (the one who makes the
    /// jump will call @a outputDestructActions() to write destruct actions)
    ///
    /// \see    Instruction, TrContext
    class Scope
    {
    public:
        Scope(TrContext& context, const Location& loc);
        ~Scope();

        /// Returns the parent context of this scope
        TrContext& context() const;

        /// Getter for the current stack of instructions
        /// Note that at different points in the scope we might have zero instructions, or different instructions stacks
        const vector<Instruction*>& instructionsStack() const;
        vector<Instruction*>& instructionsStack();

        /// Adds an scope destruct action
        void addScopeDestructAction(Node* destructAction);

        /// Translate the destruct actions corresponding to this node
        void outputDestructActions();

    protected:
        TrContext& context_;
        vector<Instruction*> instructionsStack_;
        NodeVector destructActions_;
        Location location_;
    };
}}
