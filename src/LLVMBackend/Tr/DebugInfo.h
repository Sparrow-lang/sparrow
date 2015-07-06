#pragma once

#include <Nest/Frontend/Location.h>

#include "LlvmBuilder.h"

#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#include <llvm/IR/DIBuilder.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

FWD_CLASS1(Nest, DynNode);
FWD_CLASS1(Nest, SourceCode);
FWD_CLASS1(Feather, Decl);
FWD_CLASS1(Feather, Function);

namespace LLVMB { namespace Tr
{
    using Nest::Location;

    /// Helper class used to generate debug information for the translated compile unit.
    ///
    /// This is based on the functionality from the CLang compiler.
    class DebugInfo
    {
    public:
        DebugInfo(llvm::Module& module, const string& mainFilename);
        ~DebugInfo();

        /// Finalize the generation of debug information for this program
        void finalize();

        void emitLocation(LlvmBuilder& builder, const Location& loc, bool takeStart = true);

        void emitFunctionStart(LlvmBuilder& builder, Feather::Function* fun, llvm::Function* llvmFun);
        void emitFunctionEnd(LlvmBuilder& builder, const Location& loc);

        void emitLexicalBlockStart(LlvmBuilder& builder, const Location& loc);
        void emitLexicalBlockEnd(LlvmBuilder& builder, const Location& loc);

    private:
        void createCompileUnit(const string& mainFilename);

        void setLocation(const Location& loc);

        llvm::DIFile getOrCreateFile(const Location& loc);

    private:
        /// The object used to build the debug information for the generated program
        llvm::DIBuilder diBuilder_;

        /// Debug information for the compile-unit - this corresponds to the whole program
        llvm::DICompileUnit compileUnit_;

        /// Stack of current nested lexical blocks
        vector<llvm::TrackingVH<llvm::MDNode> > lexicalBlockStack_;

        /// Stack of lexical block region count at start of each function
        /// Starting a function will push an element, ending it will pop an element
        vector<unsigned> regionCountAtFunStartStack_;

        /// Map from declarations to the corresponding metadata nodes
        llvm::DenseMap<const Feather::Decl*, llvm::WeakVH> regionMap_;

        /// Map containing the filename nodes - the filenames are represented by the SourceCode pointer
        llvm::DenseMap<const Nest::SourceCode*, llvm::DIFile> filenameCache_;

        /// The current location for which we set debug information
        Location curLoc_;
        /// The location that we previously written debug information for
        Location prevLoc_;
    };
}}
