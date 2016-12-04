
# Add the bison rule
find_package(BISON)
BISON_TARGET(SparrowParser Grammar/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.cpp)

# The source files
SET(sourceFiles
    "StdInc.h"
    "StdInc.cpp"
    "SparrowFrontend.h"
    "SparrowFrontend.cpp"
    "SparrowSourceCode.h"
    "SparrowSourceCode.cpp"
    "UserDefinedSourceCode.h"
    "UserDefinedSourceCode.cpp"
    "NodeCommonsH.h"
    "NodeCommonsCpp.h"
    "SparrowFrontendTypes.h"
    "SparrowFrontendTypes.cpp"
    "CtApiFunctions.h"
    "CtApiFunctions.cpp"
    "Mods.h"
    "Mods.cpp"
    "IntMods.h"
    "IntMods.cpp"
    "SprDebug.cpp"
    "Grammar/parser.yy"
    "Grammar/parserDefs.spr"
    "Grammar/scanner.spr"
    "Grammar/scannerIf.spr"
    "Grammar/scannerIf.ll"
    "Grammar/Scanner.h"
    "Grammar/Scanner.cpp"
    "Nodes/SparrowNodes.h"
    "Nodes/SparrowNodes.cpp"
    "Nodes/SparrowNodes_Exp.cpp"
    "Nodes/SparrowNodes_Decl.cpp"
    "Nodes/SparrowNodes_Module.cpp"
    "Nodes/SparrowNodesAccessors.h"
    "Nodes/SparrowNodesAccessors.cpp"
    "Nodes/SprProperties.h"
    "Nodes/SprProperties.cpp"
    "Nodes/Builder.h"
    "Nodes/Builder.cpp"
    "Nodes/Decls/AccessType.h"
    "Helpers/ForEachNodeInNodeList.h"
    "Helpers/SprTypeTraits.h"
    "Helpers/SprTypeTraits.cpp"
    "Helpers/StdDef.h"
    "Helpers/StdDef.cpp"
    "Helpers/DeclsHelpers.h"
    "Helpers/DeclsHelpers.cpp"
    "Helpers/Generics.h"
    "Helpers/Generics.cpp"
    "Helpers/CommonCode.h"
    "Helpers/CommonCode.cpp"
    "Helpers/Convert.h"
    "Helpers/Convert.cpp"
    "Helpers/Overload.h"
    "Helpers/Overload.cpp"
    "Helpers/Ct.h"
    "Helpers/Ct.cpp"
    "Helpers/QualifiedId.h"
    "Helpers/QualifiedId.cpp"
    "Helpers/Impl/Callable.h"
    "Helpers/Impl/Callable.cpp"
    "Helpers/Impl/FunctionCallable.h"
    "Helpers/Impl/FunctionCallable.cpp"
    "Helpers/Impl/GenericCallable.h"
    "Helpers/Impl/GenericCallable.cpp"
    "Helpers/Impl/ClassCtorCallable.h"
    "Helpers/Impl/ClassCtorCallable.cpp"
    "Helpers/Impl/ConceptCallable.h"
    "Helpers/Impl/ConceptCallable.cpp"
)

if(BOOSTSTRAP_SPARROW)
    message(STATUS "Using SparrowCompiler to generate the scanner object")
    SPARROW_TARGET(SparrowScanner
        Grammar/scannerIf.spr
        ${CMAKE_CURRENT_SOURCE_DIR}/Grammar/scannerIf.o
        COMPILE_FLAGS "-dump-assembly -fno-colors -fno-main -c"
        DEPENDS Grammar/scanner.spr Grammar/parserDefs.spr
    )
else()
    message(STATUS "Using LLVM to generate the scanner object")
    LLVMASM_TARGET(SparrowScanner
        Grammar/scannerIf.ll
        ${CMAKE_CURRENT_SOURCE_DIR}/Grammar/scannerIf.o
    )
endif()


# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
INCLUDE_DIRECTORIES( "${CMAKE_CURRENT_BINARY_DIR}" )
ADD_DEFINITIONS( "-DINSIDE_SPARROWFRONTEND" )
ADD_MSVC_PRECOMPILED_HEADER("StdInc.h" "StdInc.cpp" sourceFiles)
ADD_SOURCES_TO_GROUPS(sourceFiles)
SOURCE_GROUP("Grammar"
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.cpp
)

ADD_LIBRARY( SparrowFrontend
    ${sourceFiles}
    ${BISON_SparrowParser_OUTPUTS}
    ${SparrowScanner_OUTPUTS}
)
TARGET_LINK_LIBRARIES(SparrowFrontend ${REQ_LLVM_LIBRARIES})
TARGET_LINK_LIBRARIES(SparrowFrontend ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY})
