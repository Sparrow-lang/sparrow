#pragma once

#include <Nest/Intermediate/TypeRef.h>

FWD_STRUCT1(Nest, Node);

typedef struct Nest_SourceCode SourceCode;
typedef struct Nest_Backend Backend;

using Nest::Node;

/// Function that initializes the backed
typedef void (*FBackendInit)(Backend* backend, const char* mainFilename);
/// Function that does the backend processing and transforms the intermediate code into machine code.
typedef void (*FBackendGenerateMachineCode)(Backend* backend, const SourceCode* code);
/// Function that links a list of object files that were compiled with 'compile' method
typedef void (*FBackendLink)(Backend* backend, const char* outFilename);
/// Function that processes the given node at compile-time.
/// This is called for declarations to introduce them into the CT module
typedef void (*FBackendCtProcess)(Backend* backend, Node* node);
/// Function that evaluates the given node at compile-time. If the result isn't null, the node should be replaced with the returned node
/// This should be called for expression that have a (ct) storage type
typedef Node* (*FBackendCtEvaluate)(Backend* backend, Node* node);
/// Function that gets the size of the given type (in bytes)
typedef unsigned int (*FBackendSizeOf)(Backend* backend, TypeRef type);
/// Function that gets the alignment of the given type (in bytes)
typedef unsigned int (*FBackendAlignmentOf)(Backend* backend, TypeRef type);
/// Function that can register a CT API function
/// The given function will be added to the CT module, and the programs can call it
typedef void (*FBackendCtApiRegisterFun)(Backend* backend, const char* name, void* funPtr);

/// Type that describes a backend
/// As there aren't multiple backend of the same type, we don't have backend kinds.
/// This mainly contains a list of the functions to be called to do the backend job.
struct Nest_Backend {
    const char* name;
    const char* description;

    FBackendInit init;
    FBackendGenerateMachineCode generateMachineCode;
    FBackendLink link;
    FBackendCtProcess ctProcess;
    FBackendCtEvaluate ctEvaluate;
    FBackendSizeOf sizeOf;
    FBackendAlignmentOf alignmentOf;
    FBackendCtApiRegisterFun ctApiRegisterFun;
};
typedef struct Nest_Backend Nest_Backend;
typedef struct Nest_Backend Backend;


// Registers the given backend to the list of our backends
int Nest_registerBackend(Backend* backend);

/// Returns the number of registered backends
int Nest_getNumBackends();

/// Get the backend with the specified index
Backend* Nest_getBackend(int idx);
