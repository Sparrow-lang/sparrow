#include "StdInc.h"
#include "BackendMock.hpp"

#include "Feather/Utils/FeatherUtils.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

#include "Nest/Utils/cppif/Type.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"

using namespace Nest;

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

BackendMock::~BackendMock() = default;

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
Node* BackendMock::CtEvaluate(Nest_Backend* backend, Node* node) {
    if (!Nest_semanticCheck(node))
        return node;
    if (node->nodeKind == Feather::CtValueExp::staticKind())
        return node;
    else {
        // Return a value appropriate for the given type
        auto dataSize = int(SizeOf(backend, node->type));
        Nest::StringRefM data{dataSize};
        // Fill the data with a char
        char ch = '#';
        for (int i = 0; i < dataSize; i++)
            data.begin[i] = ch;
        return Feather::CtValueExp::create(node->location, node->type, data);
    }
}
unsigned int BackendMock::SizeOf(Nest_Backend* /*backend*/, TypeRef type) {
    auto node = Nest::Type(type).referredNode();
    if (node) {
        StringRef nativeName = node.getPropertyDefaultString(propNativeName, StringRef());
        if (nativeName == "i1")
            return 1;
        if (nativeName && *nativeName.begin == 'i') {
            nativeName.begin++;
            int numBits = atoi(nativeName.begin);
            return numBits/8;
        }
    }
    return sizeof(TypeRef); // otherwise, just return the size of a type
}
unsigned int BackendMock::AlignmentOf(Nest_Backend* /*backend*/, TypeRef /*type*/) {
    return 1; // byte alignment
}
void BackendMock::CtApiRegisterFun(Nest_Backend* /*backend*/, const char* /*name*/, void* /*funPtr*/) {
    // Nothing to do
}
