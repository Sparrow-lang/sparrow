
# General Cmake settings
cmake_minimum_required(VERSION 3.0.0)
SET( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )           # allow more human readable "if then else" constructs
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Try to find an external Sparrow executable
find_program(SPARROW_EXECUTABLE_EXT NAMES SparrowCompiler DOC "path to the SparrowCompiler executable (external)")
# Try to find an external llc executable
find_program(LLC_EXECUTABLE_EXT NAMES llc spr-llc DOC "path to the llc executable (external)")

MACRO(GROUP_NAME_FROM_PATH filePath resultVar)
    IF(MSVC OR APPLE)
        GET_FILENAME_COMPONENT(dir ${filePath} PATH)
        IF(dir STREQUAL "")
            SET(dir "\\")
        ENDIF()
        STRING(REPLACE "/" "\\" ${resultVar} "${dir}")
        #MESSAGE(STATUS "File folder: ${${resultVar}} - ${f}")
    ENDIF()
ENDMACRO(GROUP_NAME_FROM_PATH)

MACRO(ADD_SOURCES_TO_GROUPS sourcesVar)
    IF(MSVC OR APPLE)
        FOREACH(f ${${sourcesVar}})
            GROUP_NAME_FROM_PATH(${f} dirR)
            SOURCE_GROUP(${dirR} FILES ${f})
        ENDFOREACH()
    ENDIF()
ENDMACRO(ADD_SOURCES_TO_GROUPS)

MACRO(ADD_MSVC_PRECOMPILED_HEADER pchHeader pchSource sourcesVar)
    IF(MSVC)
        GET_FILENAME_COMPONENT(pchBasename ${pchHeader} NAME_WE)
        SET(pchBinary "$(IntDir)/${pchBasename}.pch")
        SET(sources ${${sourcesVar}})

        GET_FILENAME_COMPONENT(pchSourceFull ${pchSource} ABSOLUTE)
        FOREACH(f ${sources})
            GET_FILENAME_COMPONENT(fFull ${f} ABSOLUTE)
            IF(fFull STREQUAL pchSourceFull)
                SET_SOURCE_FILES_PROPERTIES(${f}
                    PROPERTIES COMPILE_FLAGS "/Yc\"${pchHeader}\" /Fp\"${pchBinary}\"" OBJECT_OUTPUTS "${pchBinary}")
            ELSE()
                SET_SOURCE_FILES_PROPERTIES(${f}
                    PROPERTIES COMPILE_FLAGS "/Yu\"${pchHeader}\" /FI\"${pchHeader}\" /Fp\"${pchBinary}\""
                    OBJECT_DEPENDS "${pchBinary}")
            ENDIF()
        ENDFOREACH()
    ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

macro(SPARROW_TARGET Name Input Output)
    set(SPARROW_TARGET_outputs "${Output}")
    set(SPARROW_EXECUTABLE_opts "")

    set(SPARROW_TARGET_PARAM_OPTIONS)
    set(SPARROW_TARGET_PARAM_ONE_VALUE_KEYWORDS
        COMPILE_FLAGS
    )
    set(SPARROW_TARGET_MULTI_VALUE_KEYWORDS
        DEPENDS
    )

    cmake_parse_arguments(
        SPARROW_TARGET_ARG
        "${SPARROW_TARGET_PARAM_OPTIONS}"
        "${SPARROW_TARGET_PARAM_ONE_VALUE_KEYWORDS}"
        "${SPARROW_TARGET_MULTI_VALUE_KEYWORDS}"
        ${ARGN}
    )

    set(SPARROW_TARGET_usage "SPARROW_TARGET(<Name> <Input> <Output> [COMPILE_FLAGS <string>] [DEPENDS <files>]")

    if(NOT "${SPARROW_TARGET_ARG_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(SEND_ERROR ${SPARROW_TARGET_usage})
    else()
        if(NOT "${SPARROW_TARGET_ARG_COMPILE_FLAGS}" STREQUAL "")
            set(SPARROW_EXECUTABLE_opts "${SPARROW_TARGET_ARG_COMPILE_FLAGS}")
            separate_arguments(SPARROW_EXECUTABLE_opts)
        endif()

        add_custom_command(OUTPUT ${SPARROW_TARGET_outputs}
            COMMAND ${SPARROW_EXECUTABLE_EXT} ${SPARROW_EXECUTABLE_opts} -o ${Output} ${Input}
            VERBATIM
            DEPENDS ${Input} ${SPARROW_TARGET_ARG_DEPENDS}
            COMMENT "[SPARROW][${Name}] Building Sparrow object ${Output}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

        set(${Name}_DEFINED TRUE)
        set(${Name}_OUTPUTS ${Output})
        set(${Name}_INPUT ${Input})
        set(${Name}_COMPILE_FLAGS ${SPARROW_EXECUTABLE_opts})
    endif()
endmacro()

macro(LLVMASM_TARGET Name Input Output)
    set(LLVMASM_TARGET_outputs "${Output}")
    set(LLVMASM_EXECUTABLE_opts "")

    set(LLVMASM_TARGET_PARAM_OPTIONS)
    set(LLVMASM_TARGET_PARAM_ONE_VALUE_KEYWORDS
        COMPILE_FLAGS
    )
    set(LLVMASM_TARGET_MULTI_VALUE_KEYWORDS
        DEPENDS
    )

    cmake_parse_arguments(
        LLVMASM_TARGET_ARG
        "${LLVMASM_TARGET_PARAM_OPTIONS}"
        "${LLVMASM_TARGET_PARAM_ONE_VALUE_KEYWORDS}"
        "${LLVMASM_TARGET_MULTI_VALUE_KEYWORDS}"
        ${ARGN}
    )

    set(LLVMASM_TARGET_usage "LLVMASM_TARGET(<Name> <Input> <Output> [COMPILE_FLAGS <string>] [DEPENDS <files>]")

    if(NOT "${LLVMASM_TARGET_ARG_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(SEND_ERROR ${LLVMASM_TARGET_usage})
    else()
        if(NOT "${LLVMASM_TARGET_ARG_COMPILE_FLAGS}" STREQUAL "")
            set(LLVMASM_EXECUTABLE_opts "${LLVMASM_TARGET_ARG_COMPILE_FLAGS}")
            separate_arguments(LLVMASM_EXECUTABLE_opts)
        endif()

        add_custom_command(OUTPUT ${LLVMASM_TARGET_outputs}
            COMMAND ${LLC_EXECUTABLE_EXT} --filetype=obj ${LLVMASM_EXECUTABLE_opts} -o ${Output} ${Input}
            VERBATIM
            DEPENDS ${Input} ${LLVMASM_TARGET_ARG_DEPENDS}
            COMMENT "[LLVM][${Name}] Building object ${Output}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

        set(${Name}_DEFINED TRUE)
        set(${Name}_OUTPUTS ${Output})
        set(${Name}_INPUT ${Input})
        set(${Name}_COMPILE_FLAGS ${LLVMASM_EXECUTABLE_opts})
    endif()
endmacro()

# MACRO(BISON_GENERATOR inputGrammarFile generatedSourcesVar)
# 	IF(BISON_EXECUTABLE)
#         # Get the running directory - the directory of the bison executable
# 		GET_FILENAME_COMPONENT(runDir ${BISON_EXECUTABLE} PATH)
#
#         # The absolute path of the input
# 		GET_FILENAME_COMPONENT(inputGrammarFileAbs ${inputGrammarFile} ABSOLUTE)
#
#         # The outputs
#         SET(outHeader ${CMAKE_CURRENT_BINARY_DIR}/${inputGrammarFile}.h)
#         SET(outCpp ${CMAKE_CURRENT_BINARY_DIR}/${inputGrammarFile}.cpp)
#         SET(${generatedSourcesVar} ${outCpp} ${outHeader})
#
#         # Add a custom command to invoke bison
# 		ADD_CUSTOM_COMMAND(
# 			OUTPUT  ${${generatedSourcesVar}}
# 			DEPENDS ${inputGrammarFile}
# 			COMMAND ${BISON_EXECUTABLE}
# 			ARGS -o"${outCpp}" --defines="${outHeader}" ${inputGrammarFileAbs}
# 			WORKING_DIRECTORY ${runDir}
#             COMMENT "Generating parser with bison for '${inputGrammarFile}'"
# 		)
#
#         # Set the right properties for the generated files
# 		SET_SOURCE_FILES_PROPERTIES( ${${generatedSourcesVar}} GENERATED)
#         GROUP_NAME_FROM_PATH(${inputGrammarFile} groupName)
#         SOURCE_GROUP(${groupName} FILES ${${generatedSourcesVar}})
# 	ELSE(BISON_EXECUTABLE)
# 		MESSAGE(SEND_ERROR "Can't find bison program; please define BISON_EXECUTABLE")
# 	ENDIF(BISON_EXECUTABLE)
# ENDMACRO(BISON_GENERATOR)
#
# MACRO(FLEX_GENERATOR inputGrammarFile generatedSourcesVar)
# 	IF(FLEX_EXECUTABLE)
#         # Get the running directory - the directory of the bison executable
# 		GET_FILENAME_COMPONENT(runDir ${FLEX_EXECUTABLE} PATH)
#
#         # The absolute path of the input
# 		GET_FILENAME_COMPONENT(inputGrammarFileAbs ${inputGrammarFile} ABSOLUTE)
#
#         # The outputs
#         SET(outCpp ${CMAKE_CURRENT_BINARY_DIR}/${inputGrammarFile}.cpp)
#         SET(${generatedSourcesVar} ${outCpp})
#
#         # Add a custom command to invoke bison
# 		ADD_CUSTOM_COMMAND(
# 			OUTPUT  ${${generatedSourcesVar}}
# 			DEPENDS ${inputGrammarFile}
# 			COMMAND ${FLEX_EXECUTABLE}
# 			ARGS -o"${outCpp}" ${inputGrammarFileAbs}
# 			WORKING_DIRECTORY ${runDir}
#             COMMENT "Generating scanner with flex for '${inputGrammarFile}'"
# 		)
#
#         # Set the right properties for the generated files
# 		SET_SOURCE_FILES_PROPERTIES( ${${generatedSourcesVar}} GENERATED)
#         GROUP_NAME_FROM_PATH(${inputGrammarFile} groupName)
#         SOURCE_GROUP(${groupName} FILES ${${generatedSourcesVar}})
# 	ELSE(FLEX_EXECUTABLE)
# 		MESSAGE(SEND_ERROR "Can't find flex program; please define FLEX_EXECUTABLE")
# 	ENDIF(FLEX_EXECUTABLE)
# ENDMACRO(FLEX_GENERATOR)
