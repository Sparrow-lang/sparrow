#include "StdInc.h"
#include "BackendMock.hpp"

BackendMock::BackendMock()
    : Nest_Backend{"mockBackend", "dummy backend to use in unit testing"} {

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

void BackendMock::Init(Nest_Backend* /*backend*/, const char* /*mainFilename*/) {
    // Nothing to do
}
void BackendMock::GenerateMachineCode(Nest_Backend* /*backend*/, const Nest_SourceCode* /*code*/) {
    // Nothing to do
}
void BackendMock::Link(Nest_Backend* /*backend*/, const char* /*outFilename*/) {
    // Nothing to do
}
void BackendMock::CtProcess(Nest_Backend* /*backend*/, Node* /*node*/) {
    // Nothing to do
}
Node* BackendMock::CtEvaluate(Nest_Backend* /*backend*/, Node* node) {
    return node; // Transparent evaluation
}
unsigned int BackendMock::SizeOf(Nest_Backend* /*backend*/, TypeRef /*type*/) {
    return sizeof(TypeRef); // aways the size of a type
}
unsigned int BackendMock::AlignmentOf(Nest_Backend* /*backend*/, TypeRef /*type*/) {
    return 1; // byte alignment
}
void BackendMock::CtApiRegisterFun(Nest_Backend* /*backend*/, const char* /*name*/, void* /*funPtr*/) {
    // Nothing to do
}
