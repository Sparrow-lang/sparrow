
# General Cmake settings
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.8 )
SET( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )           # allow more human readable "if then else" constructs
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
