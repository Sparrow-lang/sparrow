# Top-Level CMakeLists.txt

CMAKE_MINIMUM_REQUIRED( VERSION 2.8.8 )
SET(RootDir ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_MODULE_PATH ${RootDir})
INCLUDE(Macros.cmake)

SET(OutDir ${CMAKE_CURRENT_SOURCE_DIR}/.out)

SET(EXECUTABLE_OUTPUT_PATH ${OutDir})
SET(LIBRARY_OUTPUT_PATH ${OutDir})


PROJECT( SparrowCompiler )
SET( ${PROJECT_NAME}_MAJOR_VERSION 2 )
SET( ${PROJECT_NAME}_MINOR_VERSION 9 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 3 )

SET( BISON_EXECUTABLE "" CACHE PATH "The path of the Bison 3.0 executable" )
SET( FLEX_EXECUTABLE "" CACHE PATH "The path of the Flex executable" )
SET( FLEX_INCLUDE_DIR "" CACHE PATH "The path of the Flex include dir, where FlexLexer.h can be found" )

# DEPENDENCIES: this code has to be placed before adding any library or
# executable so that these are linked properly against the dependencies
IF( DEFINED ${PROJECT_NAME}_DEPENDS OR DEFINED BUILD_WITH OR DEFINED LINK_WITH )
    # load macro
    IF( NOT EXISTS "${CMAKE_MODULE_PATH}/MacroCheckDeps.cmake" )
        MESSAGE( FATAL_ERROR
            "\nSorry, could not find MacroCheckDeps.cmake...\n"
            "Please set CMAKE_MODULE_PATH correctly with: "
            "cmake -DCMAKE_MODULE_PATH=<path_to_cmake_modules>" )
    ENDIF()
    INCLUDE( "${CMAKE_MODULE_PATH}/MacroCheckDeps.cmake" )
    CHECK_DEPS()
ENDIF()


# Find the boost library
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
SET(BOOST_LIBRARYDIR "" CACHE PATH "Path to Boost libraries")
find_package( Boost 1.55.0 COMPONENTS filesystem system program_options timer REQUIRED )
message(STATUS "Boost include: ${Boost_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES( "${Boost_INCLUDE_DIRS}" )
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

IF(MSVC)
    find_package( LLVM REQUIRED )
    if( NOT LLVM_FOUND )
        message(FATAL_ERROR "LLVM package can't be found. Set CMAKE_PREFIX_PATH variable to LLVM's installation prefix.")
    endif()
    SET( LLVM_INCLUDE_DIR  ${LLVM_INCLUDE_DIRS})
    SET( LLVM_LIB_DIR  ${LLVM_LIBRARY_DIRS})
    SET( LLVM_BIN_DIR  ${LLVM_TOOLS_BINARY_DIR})
    include_directories( ${LLVM_INCLUDE_DIRS} )
    link_directories( ${LLVM_LIBRARY_DIRS} )
ELSE()
    SET( LLVM_DIR  "" CACHE PATH "The root directory for LLVM libraries" )
    SET( LLVM_INCLUDE_DIR  "" CACHE PATH "Include path for LLVM libraries" )
    SET( LLVM_LIB_DIR  "" CACHE PATH "Library path for LLVM libraries" )
    SET( LLVM_BIN_DIR  "" CACHE PATH "Library path for LLVM libraries" )
    IF ( NOT LLVM_INCLUDE_DIR AND LLVM_DIR )
        SET(LLVM_INCLUDE_DIR "${LLVM_DIR}/include")
    ENDIF()
    IF ( NOT LLVM_LIB_DIR AND LLVM_DIR )
        SET(LLVM_LIB_DIR "${LLVM_DIR}/lib")
    ENDIF()
    IF ( NOT LLVM_BIN_DIR AND LLVM_DIR )
        SET(LLVM_BIN_DIR "${LLVM_DIR}/bin")
    ENDIF()
    IF( NOT EXISTS "${LLVM_INCLUDE_DIR}/llvm/Pass.h" )
        MESSAGE( FATAL_ERROR
            "\nSorry, could not find LLVM's Pass.h\n"
            "Make sure that LLVM_DIR or LLVM_INCLUDE_DIR is correctly set\n")
    ENDIF()
    IF( NOT EXISTS "${LLVM_LIB_DIR}" )
        MESSAGE( FATAL_ERROR
            "\nSorry, could not find llvm library dir\n"
            "Make sure that LLVM_DIR or LLVM_LIB_DIR is correctly set\n")
    ENDIF()
    IF( NOT EXISTS "${LLVM_BIN_DIR}/llc" AND NOT EXISTS "${LLVM_BIN_DIR}/llc.exe" )
        MESSAGE( FATAL_ERROR
            "\nSorry, could not find llvm binary dir\n"
            "Make sure that LLVM_DIR or LLVM_BIN_DIR is correctly set\n")
    ENDIF()
ENDIF()
message(STATUS "LLVM include: ${LLVM_INCLUDE_DIR}")
message(STATUS "LLVM libs: ${LLVM_LIB_DIR}")
message(STATUS "LLVM binaries: ${LLVM_BIN_DIR}")
LINK_DIRECTORIES(${LLVM_LIB_DIR})
INCLUDE_DIRECTORIES( ${LLVM_INCLUDE_DIR} )


# User defined settings (to help in finding the Boost libraries)
SET( SC_DISABLE_STL_DEBUGGING  "OFF" CACHE BOOL "Check this to compile with _SECURE_SCL=0 and _HAS_ITERATOR_DEBUGGING=0" )

# Project options
IF( NOT CMAKE_BUILD_TYPE )      # set default cmake build type to RelWithDebInfo (None Debug Release RelWithDebInfo MinSizeRel)
    SET( CMAKE_BUILD_TYPE "RelWithDebInfo" )
ENDIF()
IF( CMAKE_INSTALL_PREFIX STREQUAL "/usr/local" )        # set default install prefix to project root directory
    SET( CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}" )
ENDIF()

# Compilation flags
IF(MSVC)
    ADD_DEFINITIONS( "-W4" )        # Warning level 4
    ADD_DEFINITIONS( -DWIN32 )
    ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE )
    IF(SC_DISABLE_STL_DEBUGGING)
        ADD_DEFINITIONS( -D_SECURE_SCL=0 -D_HAS_ITERATOR_DEBUGGING=0 )
    ENDIF()
ELSEIF(APPLE)
    ADD_DEFINITIONS( -D__STDC_LIMIT_MACROS=1 )
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=gnu++0x" )
    ADD_DEFINITIONS( -Wall )                    # All warnings...
    ADD_DEFINITIONS( -Wno-c++11-extensions )    # ... except C++11 extensions warnings
    ADD_DEFINITIONS( -Wno-deprecated )          # ... and except deprecated functions
    ADD_DEFINITIONS( -Wno-undefined-bool-conversion ) # ... and except undefined bool conversions
    ADD_DEFINITIONS( -Wno-typedef-redefinition )   # redefinition of typedef '...' is a C11 feature
    ADD_DEFINITIONS( -Wno-deprecated )
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=gnu++0x" )
ENDIF()


ADD_SUBDIRECTORY( src/Nest )
ADD_SUBDIRECTORY( src/Feather )
ADD_SUBDIRECTORY( src/LLVMBackend )
ADD_SUBDIRECTORY( src/SparrowFrontend )
ADD_SUBDIRECTORY( src/SparrowCompiler )
