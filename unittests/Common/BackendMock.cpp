#include "StdInc.h"
#include "BackendMock.hpp"

BackendMock::BackendMock()
    : Backend{"mockBackend", "dummy backend to use in unittesting"} {

    init = &BackendMock::Init;
    generateMachineCode = &BackendMock::GenerateMachineCode;
    link = &BackendMock::Link;
    ctProcess = &BackendMock::CtProcess;
    ctEvaluate = &BackendMock::CtEvaluate;
    sizeOf = &BackendMock::SizeOf;
    alignmentOf = &BackendMock::AlignmentOf;
    ctApiRegisterFun = &BackendMock::CtApiRegisterFun;
}

BackendMock::~BackendMock() {}

void BackendMock::Init(Backend* /*backend*/, const char* /*mainFilename*/) {
    // Nothing to do
}
void BackendMock::GenerateMachineCode(Backend* /*backend*/, const SourceCode* /*code*/) {
    // Nothing to do
}
void BackendMock::Link(Backend* /*backend*/, const char* /*outFilename*/) {
    // Nothing to do
}
void BackendMock::CtProcess(Backend* /*backend*/, Node* /*node*/) {
    // Nothing to do
}
Node* BackendMock::CtEvaluate(Backend* /*backend*/, Node* node) {
    return node; // Transparent evaluation
}
unsigned int BackendMock::SizeOf(Backend* /*backend*/, TypeRef /*type*/) {
    return sizeof(TypeRef); // aways the size of a type
}
unsigned int BackendMock::AlignmentOf(Backend* /*backend*/, TypeRef /*type*/) {
    return 1; // byte alignment
}
void BackendMock::CtApiRegisterFun(Backend* /*backend*/, const char* /*name*/, void* /*funPtr*/) {
    // Nothing to do
}
