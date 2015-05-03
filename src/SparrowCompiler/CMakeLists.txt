
# The source files
SET(sourceFiles
    "StdInc.cpp"
    "StdInc.h"
    "SparrowCompiler.cpp"
    "Settings.h"
    "Settings.cpp"
)

# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
ADD_MSVC_PRECOMPILED_HEADER("StdInc.h" "StdInc.cpp" sourceFiles)
ADD_SOURCES_TO_GROUPS(sourceFiles)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})
ADD_EXECUTABLE( SparrowCompiler ${sourceFiles} )
TARGET_LINK_LIBRARIES(SparrowCompiler Nest Feather LLVMBackend SparrowFrontend)
TARGET_LINK_LIBRARIES(SparrowCompiler ${LLVM_LIBS})
IF(APPLE)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVM-3.5)
    TARGET_LINK_LIBRARIES(SparrowCompiler boost_timer)
ELSE()
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMCore)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMJIT)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMAsmParser)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMBitWriter)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMipo)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMLinker)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMX86Info)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMX86Desc)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMX86CodeGen)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMExecutionEngine)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMVectorize)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMX86Desc)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMX86Info)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMObject)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMBitReader)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMX86AsmPrinter)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMAsmPrinter)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMMCParser)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMSelectionDAG)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMCodeGen)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMScalarOpts)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMInstCombine)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMTransformUtils)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMipa)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMAnalysis)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMTarget)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMCore)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMMC)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMX86Utils)
    TARGET_LINK_LIBRARIES(SparrowCompiler LLVMSupport)

ENDIF()


# Copy llvm binaries in the output directory
# If the binary dir depends on configuration, take Release
STRING(REPLACE "$(Configuration)" "Release" llvmBinDirRel ${LLVM_BIN_DIR})
FILE(GLOB llvmBinaries "${llvmBinDirRel}/*")

foreach(llvmFile ${llvmBinaries})
    get_filename_component(fileName ${llvmFile} NAME)
    add_custom_command(TARGET SparrowCompiler POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${llvmFile}"
            "$<TARGET_FILE_DIR:SparrowCompiler>/llvm/${fileName}")
endforeach()

# Copy the LLVM dynamic library
IF(APPLE)
    add_custom_command(TARGET SparrowCompiler POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LLVM_LIB_DIR}/libLLVM-3.5.dylib"
            "$<TARGET_FILE_DIR:SparrowCompiler>/lib/libLLVM-3.5.dylib")
ENDIF()
