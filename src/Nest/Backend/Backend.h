#pragma once

#include <Nest/Intermediate/TypeRef.h>

#include <vector>

FWD_CLASS1(Nest, Node);
FWD_CLASS1(Nest, SourceCode);

namespace Nest
{
    /// Base class for the backend component of the compiler
    /// The backend is responsible for translating intermediate code into machine code
    class Backend
    {
    public:
        Backend();
        virtual ~Backend();

        /// Called to initialzie the backed
        virtual void init(const string& mainFilename) = 0;

        /// Does the backend processing and transforms the intermediate code into machine code.
        virtual void generateMachineCode(SourceCode& code) = 0;

        /// Links a list of object files that were compiled with 'compile' method
        virtual void link(const string& outFilename) = 0;

		/// Processes the given node at compile-time.
        /// This is called for declarations to introduce them into the CT module
        virtual void ctProcess(Node* node) = 0;

		/// Evaluates the given node at compile-time. If the result isn't null, the node should be replaced with the returned node
		/// This should be called for expression that have a (ct) storage type
        virtual Node* ctEvaluate(Node* node) = 0;

        /// Get the size of the given type (in bytes)
        virtual size_t sizeOf(TypeRef type) = 0;
        /// Get the alignment of the given type (in bytes)
        virtual size_t alignmentOf(TypeRef type) = 0;

        /// Register a CT API function
        /// This function will be added to the CT module, and the Sparrow programs can call it
        virtual void ctApiRegisterFun(const char* name, void* funPtr) = 0;
    };
}
