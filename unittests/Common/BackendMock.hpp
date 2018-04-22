#pragma once

#include "Nest/Api/Backend.h"

struct BackendMock : Backend
{
public:
    BackendMock();
    ~BackendMock();

private:
    static void Init(Backend* backend, const char* mainFilename);
    static void GenerateMachineCode(Backend* backend, const SourceCode* code);
    static void Link(Backend* backend, const char* outFilename);
    static void CtProcess(Backend* backend, Node* node);
    static Node* CtEvaluate(Backend* backend, Node* node);
    static unsigned int SizeOf(Backend* backend, TypeRef type);
    static unsigned int AlignmentOf(Backend* backend, TypeRef type);
    static void CtApiRegisterFun(Backend* backend, const char* name, void* funPtr);
};