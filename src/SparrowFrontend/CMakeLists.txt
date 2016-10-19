
# Add the flex & bison rules
find_package(FLEX)
find_package(BISON)
BISON_TARGET(SparrowParser Grammar/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.cpp VERBOSE ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.output)
FLEX_TARGET(SparrowScanner Grammar/scanner.ll ${CMAKE_CURRENT_BINARY_DIR}/scanner.ll.cpp)
ADD_FLEX_BISON_DEPENDENCY(SparrowScanner SparrowParser)
include_directories(${FLEX_INCLUDE_DIR})

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
    "Grammar/scanner.ll"
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



# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
INCLUDE_DIRECTORIES( "${CMAKE_CURRENT_BINARY_DIR}" )
INCLUDE_DIRECTORIES( "${FLEX_INCLUDE_DIRS}" )
ADD_DEFINITIONS( "-DINSIDE_SPARROWFRONTEND" )
ADD_MSVC_PRECOMPILED_HEADER("StdInc.h" "StdInc.cpp" sourceFiles)
ADD_SOURCES_TO_GROUPS(sourceFiles)
SOURCE_GROUP("Grammar" FILES ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.hpp ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.cpp ${CMAKE_CURRENT_BINARY_DIR}/scanner.ll.cpp)

ADD_LIBRARY( SparrowFrontend ${sourceFiles} ${BISON_SparrowParser_OUTPUTS} ${FLEX_SparrowScanner_OUTPUTS} )
TARGET_LINK_LIBRARIES(SparrowFrontend ${REQ_LLVM_LIBRARIES})
TARGET_LINK_LIBRARIES(SparrowFrontend ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY})
