
# Add the flex & bison rules
find_package(FLEX)
find_package(BISON)
BISON_TARGET(SparrowParser Grammar/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.cpp VERBOSE ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.output)
FLEX_TARGET(SparrowScanner Grammar/scanner.ll ${CMAKE_CURRENT_BINARY_DIR}/scanner.ll.cpp)
ADD_FLEX_BISON_DEPENDENCY(SparrowScanner SparrowParser)
message(STATUS "Flex include: ${FLEX_INCLUDE_DIR}")
include_directories(${FLEX_INCLUDE_DIR})

# The source files
SET(sourceFiles
    "StdInc.h"
    "StdInc.cpp"
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
    "Grammar/parser.yy"
    "Grammar/scanner.ll"
    "Grammar/Scanner.h"
    "Grammar/Scanner.cpp"
    "Nodes/SparrowNodes.h"
    "Nodes/SparrowNodes.cpp"
    "Nodes/SparrowNodes_Exp.cpp"
    "Nodes/SparrowNodes_Decl.cpp"
    "Nodes/SparrowNodesAccessors.h"
    "Nodes/SparrowNodesAccessors.cpp"
    "Nodes/DynNode.h"
    "Nodes/DynNode.cpp"
    "Nodes/DynNodeVector.h"
    "Nodes/SprProperties.h"
    "Nodes/SprProperties.cpp"
    "Nodes/Builder.h"
    "Nodes/Builder.cpp"
    "Nodes/Decls/AccessType.h"
    "Nodes/Decls/SprConcept.h"
    "Nodes/Decls/SprConcept.cpp"
    "Nodes/Decls/SprVariable.h"
    "Nodes/Decls/SprVariable.cpp"
    "Nodes/Decls/SprParameter.h"
    "Nodes/Decls/SprParameter.cpp"
    "Nodes/Decls/Using.h"
    "Nodes/Decls/Using.cpp"
    "Nodes/Decls/Generic.h"
    "Nodes/Decls/Generic.cpp"
    "Nodes/Decls/GenericClass.h"
    "Nodes/Decls/GenericClass.cpp"
    "Nodes/Decls/GenericFunction.h"
    "Nodes/Decls/GenericFunction.cpp"
    "Nodes/Decls/Instantiation.h"
    "Nodes/Decls/Instantiation.cpp"
    "Nodes/Decls/InstantiationsSet.h"
    "Nodes/Decls/InstantiationsSet.cpp"
    "Mods/ModStatic.h"
    "Mods/ModStatic.cpp"
    "Mods/ModCt.h"
    "Mods/ModCt.cpp"
    "Mods/ModRt.h"
    "Mods/ModRt.cpp"
    "Mods/ModRtCt.h"
    "Mods/ModRtCt.cpp"
    "Mods/ModAutoCt.h"
    "Mods/ModAutoCt.cpp"
    "Mods/ModCtGeneric.h"
    "Mods/ModCtGeneric.cpp"
    "Mods/ModNative.h"
    "Mods/ModNative.cpp"
    "Mods/ModConvert.h"
    "Mods/ModConvert.cpp"
    "Mods/ModNoDefault.h"
    "Mods/ModNoDefault.cpp"
    "Mods/ModInitCtor.h"
    "Mods/ModInitCtor.cpp"
    "Mods/ModMacro.h"
    "Mods/ModMacro.cpp"
    "Mods/ModNoInline.h"
    "Mods/ModNoInline.cpp"
    "IntMods/IntModClassMembers.h"
    "IntMods/IntModClassMembers.cpp"
    "IntMods/IntModDtorMembers.h"
    "IntMods/IntModDtorMembers.cpp"
    "IntMods/IntModCtorMembers.h"
    "IntMods/IntModCtorMembers.cpp"
    "Helpers/ForEachNodeInNodeList.h"
    "Helpers/SprTypeTraits.h"
    "Helpers/SprTypeTraits.cpp"
    "Helpers/StdDef.h"
    "Helpers/StdDef.cpp"
    "Helpers/DeclsHelpers.h"
    "Helpers/DeclsHelpers.cpp"
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
