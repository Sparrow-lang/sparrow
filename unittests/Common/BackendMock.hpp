#pragma once

#include "Nest/Api/Backend.h"
#include "Nest/Utils/cppif/Fwd.hpp"

using Nest::TypeRef;
using Nest::Node;


struct BackendMock : Nest_Backend
{
public:
    BackendMock();
    ~BackendMock();

private:
    static void Init(Nest_Backend* backend, const char* mainFilename);
    static void GenerateMachineCode(Nest_Backend* backend, const Nest_SourceCode* code);
    static void Link(Nest_Backend* backend, const char* outFilename);
    static void CtProcess(Nest_Backend* backend, Node* node);
    static Node* CtEvaluate(Nest_Backend* backend, Node* node);
    static unsigned int SizeOf(Nest_Backend* backend, TypeRef type);
    static unsigned int AlignmentOf(Nest_Backend* backend, TypeRef type);
    static void CtApiRegisterFun(Nest_Backend* backend, const char* name, void* funPtr);
};