
# The source files
SET(sourceFiles
    "StdInc.cpp"
    "StdInc.h"
    "SparrowCompiler.cpp"
    "Settings.h"
    "Settings.cpp"
    "AstDump.h"
    "AstDump.cpp"
)

# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
ADD_MSVC_PRECOMPILED_HEADER("StdInc.h" "StdInc.cpp" sourceFiles)
ADD_SOURCES_TO_GROUPS(sourceFiles)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})
ADD_EXECUTABLE( SparrowCompiler ${sourceFiles} )
TARGET_LINK_LIBRARIES(SparrowCompiler SparrowFrontend LLVMBackend Feather Nest)
llvm_map_components_to_libnames(LLVM_LIBS
    support core irreader codegen linker
    ipo vectorize objcarcopts
    mc jit mcjit interpreter executionengine
    x86info x86desc x86codegen x86asmparser x86disassembler
    bitreader bitwriter
)
llvm_map_components_to_libnames(LLVM_EXTRA_LIBS
    asmparser asmprinter mcparser ipa analysis object transformutils
    scalaropts selectiondag target instcombine instrumentation runtimedyld
    x86asmprinter x86utils
)

message(STATUS "LLVM libraries: ${LLVM_LIBS}")
TARGET_LINK_LIBRARIES(SparrowCompiler ${LLVM_LIBS})
TARGET_LINK_LIBRARIES(SparrowCompiler boost_timer boost_chrono boost_program_options)

# Copy llvm binaries in the output directory
# Also add install targets
FILE(GLOB llvmBinaries "${CONAN_BIN_DIRS_LLVM}/*")
foreach(llvmFile ${llvmBinaries})
    get_filename_component(fileName ${llvmFile} NAME)
    add_custom_command(TARGET SparrowCompiler POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${llvmFile}"
            "$<TARGET_FILE_DIR:SparrowCompiler>/llvm/${fileName}")
    install(FILES "${llvmFile}" DESTINATION "bin/llvm" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                    WORLD_EXECUTE WORLD_READ)
endforeach()


# Copy the LLVM dynamic libraries
# Also add install targets
if(APPLE)
    foreach(llvmLib ${LLVM_LIBS})
        add_custom_command(TARGET SparrowCompiler POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LLVM_LIBRARY_DIRS}/lib${llvmLib}.dylib"
                "$<TARGET_FILE_DIR:SparrowCompiler>/")
        install(FILES "${LLVM_LIBRARY_DIRS}/lib${llvmLib}.dylib" DESTINATION lib)
    endforeach()
elseif(UNIX)
    foreach(llvmLib ${LLVM_LIBS})
        add_custom_command(TARGET SparrowCompiler POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LLVM_LIBRARY_DIRS}/lib${llvmLib}.so"
                "$<TARGET_FILE_DIR:SparrowCompiler>/lib/lib${llvmLib}.so")
        install(FILES "${LLVM_LIBRARY_DIRS}/lib${llvmLib}.so" DESTINATION lib)
    endforeach()
    foreach(llvmLib ${LLVM_EXTRA_LIBS})
        add_custom_command(TARGET SparrowCompiler POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LLVM_LIBRARY_DIRS}/lib${llvmLib}.so"
                "$<TARGET_FILE_DIR:SparrowCompiler>/lib/lib${llvmLib}.so")
        install(FILES "${LLVM_LIBRARY_DIRS}/lib${llvmLib}.so" DESTINATION lib)
    endforeach()
endif()

# Install targets for the executable and the implicit lib
install(TARGETS SparrowCompiler DESTINATION bin)
install(DIRECTORY ../../SparrowImplicitLib DESTINATION include FILES_MATCHING PATTERN *.spr)
install(DIRECTORY ../../SparrowImplicitLib DESTINATION include FILES_MATCHING PATTERN *.llvm)
