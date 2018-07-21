#pragma once

#include "Nest/Api/Location.h"
#include "Nest/Api/TypeRef.h"
#include "Nest/Api/Type.h"

#include "LlvmBuilder.h"

#include <llvm/IR/DIBuilder.h>

using Nest::Location;
using Nest::TypeRef;
using Nest::Node;

namespace LLVMB {
namespace Tr {

struct GlobalContext;

//! Helper class used to generate debug information for the translated compile unit.
//!
//! This is based on the functionality from the clang compiler.
class DebugInfo {
public:
    DebugInfo(llvm::Module& module, const string& mainFilename);
    ~DebugInfo();

    //! Finalize the generation of debug information for this program
    void finalize();

    //! Emit the current location as debug info; called for each node we are translating
    void emitLocation(LlvmBuilder& builder, const Location& loc, bool takeStart = true);

    //! Emit the debug info for the start of the function
    void emitFunctionStart(GlobalContext& ctx, LlvmBuilder& builder, Node* fun,
            llvm::Function* llvmFun, llvm::BasicBlock* bodyBlock);
    //! Called at the end of a function to fix-up debug info
    void emitFunctionEnd(LlvmBuilder& builder, const Location& loc);

    //! Emit the debug info for a parameter variable
    void emitParamVar(GlobalContext& ctx, Node* param, int idx, llvm::AllocaInst* llvmAlloca);

    //! Emit the debug info for a local variable
    void emitLocalVar(GlobalContext& ctx, Node* var, llvm::AllocaInst* llvmAlloca);

    //! Emit a debug-info for a local space
    void emitLexicalBlockStart(LlvmBuilder& builder, const Location& loc);
    //! Called at the end of a local space to fix-up debug info
    void emitLexicalBlockEnd(LlvmBuilder& builder, const Location& loc);

private:
    //! Create the debug info for a compile unit
    void createCompileUnit(const string& mainFilename);

    //! Called for each new location; check the location is the same, and if we need to emit debug
    //! info for the new location.
    void setLocation(LlvmBuilder& builder, const Location& loc);

    //! Get or create the DI for the file corresponding to the given location
    llvm::DIFile* getOrCreateFile(const Location& loc);

    //! Creates debug-info for the given type
    llvm::DIType* createDiType(GlobalContext& ctx, TypeRef type);

    //! Creates debug-info for function type
    llvm::DISubroutineType* createDiFunType(GlobalContext& ctx, TypeRef type);

    //! Creates debug-info for the given structure type
    llvm::DIType* createDiStructType(GlobalContext& ctx, TypeRef type);

private:
    //! The object used to build the debug information for the generated program
    llvm::DIBuilder diBuilder_;

    //! Debug information for the compile-unit - this corresponds to the whole program
    llvm::DICompileUnit* compileUnit_;

    //! Stack of current nested lexical blocks
    vector<llvm::DIScope*> lexicalBlockStack_;

    //! Stack of lexical block region count at start of each function
    //! Starting a function will push an element, ending it will pop an element
    vector<unsigned> regionCountAtFunStartStack_;

    //! Map containing the filename nodes - the filenames are represented by the SourceCode pointer
    llvm::DenseMap<const Nest_SourceCode*, llvm::DIFile*> filenameCache_;

    //! Map containing all the DITypes we've translated so far
    llvm::DenseMap<TypeRef, llvm::DIType*> typesMap_;

    //! The current location for which we set debug information
    Location curLoc_{};
    //! The location that we previously written debug information for
    Location prevLoc_{};
};

} // namespace Tr
} // namespace LLVMB
