
# The source files
SET(sourceFiles
    "StdInc.h"
    "StdInc.cpp"
    "LLVMBackend.h"
    "LLVMBackend.cpp"
    "LLVMSourceCode.h"
    "LLVMSourceCode.cpp"
    "Generator.h"
    "Generator.cpp"
    "Module.h"
    "Module.cpp"
    "RtModule.h"
    "RtModule.cpp"
    "CtModule.h"
    "CtModule.cpp"
    "DataLayoutHelper.h"
    "DataLayoutHelper.cpp"
    "Tr/TrContext.h"
    "Tr/TrContext.cpp"
    "Tr/Scope.h"
    "Tr/Scope.cpp"
    "Tr/Instruction.h"
    "Tr/Instruction.cpp"
    "Tr/DebugInfo.h"
    "Tr/DebugInfo.cpp"
    "Tr/TrType.h"
    "Tr/TrType.cpp"
    "Tr/TrTopLevel.h"
    "Tr/TrTopLevel.cpp"
    "Tr/TrFunction.h"
    "Tr/TrFunction.cpp"
    "Tr/TrLocal.h"
    "Tr/TrLocal.cpp"
    "Tr/LlvmBuilder.h"
)

# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
ADD_DEFINITIONS( "-DINSIDE_LLVMBACKEND" )
ADD_MSVC_PRECOMPILED_HEADER("StdInc.h" "StdInc.cpp" sourceFiles)
ADD_SOURCES_TO_GROUPS(sourceFiles)

ADD_LIBRARY( LLVMBackend ${sourceFiles} )
TARGET_LINK_LIBRARIES(LLVMBackend ${REQ_LLVM_LIBRARIES})
