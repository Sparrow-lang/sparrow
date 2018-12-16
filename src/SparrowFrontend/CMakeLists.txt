
# The source files
SET(sourceFiles
    "CtApiFunctions.cpp"
    "CtApiFunctions.h"
    "Grammar/ext.spr"
    "Grammar/layoutDecoder.spr"
    "Grammar/node.spr"
    "Grammar/Parser.cpp"
    "Grammar/Parser.h"
    "Grammar/parser.spr"
    "Grammar/parserDefs.spr"
    "Grammar/parserIf.ll"
    "Grammar/parserIf.spr"
    "Grammar/scanner.spr"
    "Grammar/token.spr"
    "Helpers/CommonCode.cpp"
    "Helpers/CommonCode.h"
    "Helpers/Convert.cpp"
    "Helpers/Convert.h"
    "Helpers/Ct.cpp"
    "Helpers/Ct.h"
    "Helpers/DeclsHelpers.cpp"
    "Helpers/DeclsHelpers.h"
    "Helpers/Generics.cpp"
    "Helpers/Generics.h"
    "Helpers/Impl/Callable.cpp"
    "Helpers/Impl/Callable.h"
    "Helpers/Impl/Intrinsics.cpp"
    "Helpers/Impl/Intrinsics.h"
    "Helpers/Overload.cpp"
    "Helpers/Overload.h"
    "Helpers/QualifiedId.cpp"
    "Helpers/QualifiedId.h"
    "Helpers/SprTypeTraits.cpp"
    "Helpers/SprTypeTraits.h"
    "Helpers/StdDef.cpp"
    "Helpers/StdDef.h"
    "IntMods.cpp"
    "IntMods.h"
    "Mods.cpp"
    "Mods.h"
    "NodeCommonsCpp.h"
    "NodeCommonsH.h"
    "Nodes/AccessType.h"
    "Nodes/Builder.cpp"
    "Nodes/Builder.h"
    "Nodes/Decl.cpp"
    "Nodes/Exp.cpp"
    "Nodes/Generics.cpp"
    "Nodes/Module.cpp"
    "Nodes/SparrowNodes.cpp"
    "Nodes/SparrowNodes.h"
    "Nodes/SprProperties.cpp"
    "Nodes/SprProperties.h"
    "Nodes/Stmt.cpp"
    "SparrowFrontend.cpp"
    "SparrowFrontend.h"
    "SparrowSourceCode.cpp"
    "SparrowSourceCode.h"
    "SprDebug.cpp"
    "src/Utils/cppif/SparrowFrontendTypes.cpp"
    "StdInc.h"
    "UserDefinedSourceCode.cpp"
    "UserDefinedSourceCode.h"
    "Utils/cppif/SparrowFrontendTypes.hpp"
)

if(BOOTSTRAP_SPARROW)
    message(STATUS "Using SparrowCompiler to generate the parser object")
    SPARROW_TARGET(SparrowParser
        ${CMAKE_CURRENT_SOURCE_DIR}/Grammar/parserIf.spr
        ${CMAKE_CURRENT_BINARY_DIR}/parserIf.o
        COMPILE_FLAGS "-dump-assembly -fno-colors -fno-main -c"
        DEPENDS Grammar/parser.spr
                Grammar/parserDefs.spr
                Grammar/ext.spr
                Grammar/layoutDecoder.spr
                Grammar/node.spr
                Grammar/scanner.spr
                Grammar/token.spr
    )
else()
    LLVMASM_TARGET(SparrowParser
        ${CMAKE_CURRENT_SOURCE_DIR}/Grammar/parserIf.ll
        ${CMAKE_CURRENT_BINARY_DIR}/parserIf.o
        COMPILE_FLAGS -relocation-model=pic
    )
endif()


# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
INCLUDE_DIRECTORIES( "${CMAKE_CURRENT_BINARY_DIR}" )
ADD_DEFINITIONS( "-DINSIDE_SPARROWFRONTEND" )
ADD_SOURCES_TO_GROUPS(sourceFiles)

ADD_LIBRARY( SparrowFrontend
    ${sourceFiles}
    ${SparrowParser_OUTPUTS}
)
TARGET_LINK_LIBRARIES(SparrowFrontend ${REQ_LLVM_LIBRARIES})
TARGET_LINK_LIBRARIES(SparrowFrontend ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY})
