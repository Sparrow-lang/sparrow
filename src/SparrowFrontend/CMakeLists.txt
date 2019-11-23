
# The source files
SET(sourceFiles
    "CtApiFunctions.cpp"
    "Grammar/ext.spr"
    "Grammar/layoutDecoder.spr"
    "Grammar/node.spr"
    "Grammar/Parser.cpp"
    "Grammar/parser.spr"
    "Grammar/parserDefs.spr"
    "Grammar/parserIf.ll"
    "Grammar/parserIf.spr"
    "Grammar/scanner.spr"
    "Grammar/token.spr"
    "Helpers/CommonCode.cpp"
    "Helpers/Ct.cpp"
    "Helpers/DeclsHelpers.cpp"
    "Helpers/Generics.cpp"
    "Helpers/Impl/Intrinsics.cpp"
    "Helpers/QualifiedId.cpp"
    "Helpers/SprTypeTraits.cpp"
    "Helpers/StdDef.cpp"
    "IntMods.cpp"
    "Mods.cpp"
    "Nodes/Builder.cpp"
    "Nodes/Decl.cpp"
    "Nodes/Exp.cpp"
    "Nodes/Generics.cpp"
    "Nodes/Module.cpp"
    "Nodes/SparrowNodes.cpp"
    "Nodes/SprProperties.cpp"
    "Nodes/Stmt.cpp"
    "Services/Callable/Callable.cpp"
    "Services/Callable/CallableHelpers.cpp"
    "Services/Callable/Callables.cpp"
    "Services/Callable/CallableServiceImpl.cpp"
    "Services/Callable/ConceptCallable.cpp"
    "Services/Callable/FunctionCallable.cpp"
    "Services/Callable/GenericDatatypeCallable.cpp"
    "Services/Callable/GenericFunctionCallable.cpp"
    "Services/Callable/GenericPackageCallable.cpp"
    "Services/Concepts/ConceptsServiceImpl.cpp"
    "Services/Convert/ConversionResult.cpp"
    "Services/Convert/ConvertServiceImpl.cpp"
    "Services/Overload/OverloadServiceImpl.cpp"
    "SparrowFrontend.cpp"
    "SparrowFrontendTypes.cpp"
    "SparrowSourceCode.cpp"
    "SprDebug.cpp"
    "UserDefinedSourceCode.cpp"
)

if(BOOTSTRAP_SPARROW)
    message(STATUS "Using SparrowCompiler to generate the parser object")
    SPARROW_TARGET(SparrowParser
        ${CMAKE_CURRENT_SOURCE_DIR}/Grammar/parserIf.spr
        ${CMAKE_CURRENT_BINARY_DIR}/parserIf.o
        COMPILE_FLAGS "-dump-assembly -fno-colors -fno-main -c -O2"
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
TARGET_LINK_LIBRARIES(SparrowFrontend ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY})

# if(BOOTSTRAP_SPARROW)
#     # Copy the generated assembly back to the source dir, so that we can commit it.
#     add_custom_command(
#         TARGET SparrowFrontend POST_BUILD
#         COMMAND
#             ${CMAKE_COMMAND} -E copy
#             ${CMAKE_CURRENT_BINARY_DIR}/Grammar/parserIf.ll
#             ${CMAKE_CURRENT_SOURCE_DIR}/Grammar/parserIf.ll)
# endif()
