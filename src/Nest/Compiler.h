#pragma once

#include <Nest/Intermediate/TypeRef.h>

FWD_CLASS1(Nest, CompilerSettings);
FWD_CLASS2(Nest,Common, DiagnosticReporter);
FWD_CLASS2(Nest,Common, ObjectFactoryReg);

typedef struct Nest_Node Node;
typedef struct Nest_SourceCode SourceCode;
typedef struct Nest_Backend Backend;
typedef struct Nest_CompilationContext CompilationContext;

namespace Nest
{
	/// Interface for the main object of the compiler platform
	class Compiler
    {
    public:
		virtual ~Compiler() {}

    // Main objects
    public:
        /// Getter for the compiler settings
        virtual CompilerSettings& settings() = 0;
        virtual const CompilerSettings& settings() const = 0;

        /// Object to report diagnostic messages
        virtual Common::DiagnosticReporter& diagnosticReporter() const = 0;

        /// The root compilation context that contains all the code compiled
        virtual CompilationContext* rootContext() const = 0;

        /// The backend object used to generate the resulting code
        virtual Backend& backend() const = 0;

    // Compilation flow
    public:
        /// Creates the backend
        virtual void createBackend(const char* mainFilename) = 0;

        /// Parse, check dependencies and the compile the given filename
        virtual void compileFile(const string& filename) = 0;

        /// Add a source code to be compiled, given the filename
        virtual void addSourceCodeByFilename(const SourceCode* orig, string filename) = 0;

        /// Add a source code to be compiled, given a qualified ID to the source code
        virtual void addSourceCodeByQid(const SourceCode* orig, vector<string> qid) = 0;
        
        /// Get the SourceCode corresponding to the given filename
        virtual const SourceCode* getSourceCodeForFilename(const string& filename) = 0;

        /// Add the given node to be queued for semantic check
        virtual void queueSemanticCheck(Node* node) = 0;

        /// Call this to CT process the given node
        virtual void ctProcess(Node* node) = 0;

        /// Call this to CT evaluate the given node
        virtual Node* ctEval(Node* node) = 0;

        /// Get the size of the given type
        virtual size_t sizeOf(TypeRef type) = 0;

        /// Get the alignment of the given type
        virtual size_t alignmentOf(TypeRef type) = 0;
    };

	/// Getter for the instance of the compiler
	Compiler& theCompiler();
}
 