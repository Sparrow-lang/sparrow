# Top-Level CMakeLists.txt

cmake_minimum_required(VERSION 3.0.0)

# Project information
project( SparrowCompiler )

# Get compiler version
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
    find_package(Git)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --long --tags --dirty --always
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE "SparrowCompiler_BUILD_VERSION"
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        set(SparrowCompiler_BUILD_VERSION 0)
    endif()
endif()
string(REGEX REPLACE
    "v([0-9]+)\\.([0-9]+).*"
    "\\1"
    SparrowCompiler_MAJOR_VERSION ${SparrowCompiler_BUILD_VERSION})
string(REGEX REPLACE
    "v([0-9]+)\\.([0-9]+).*"
    "\\2"
    SparrowCompiler_MINOR_VERSION ${SparrowCompiler_BUILD_VERSION})
string(TIMESTAMP SparrowCompiler_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP SparrowCompiler_BUILD_YEAR "%Y")

# Print configuration
message(STATUS "System       : ${CMAKE_SYSTEM}")
message(STATUS "System name  : ${CMAKE_SYSTEM_NAME}")
message(STATUS "System ver   : ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Compiler ver : ${SparrowCompiler_BUILD_VERSION}")
message(STATUS "  major ver  : ${SparrowCompiler_MAJOR_VERSION}")
message(STATUS "  minor ver  : ${SparrowCompiler_MINOR_VERSION}")
message(STATUS "Build date   : ${SparrowCompiler_BUILD_DATE}")
message(STATUS "Build year   : ${SparrowCompiler_BUILD_YEAR}")
message(STATUS)

# Configure required files
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/SparrowImplicitLib/std/compilerInfo.spr.in ${CMAKE_CURRENT_SOURCE_DIR}/SparrowImplicitLib/std/compilerInfo.spr)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/SparrowCompiler/VersionInfo.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/SparrowCompiler/VersionInfo.h)


# User passed compilation options
option(BOOSTSTRAP_SPARROW "Use system-wide SparrowCompiler to compile Sparrow files needed for the compiler" OFF)
message(STATUS "BOOSTSTRAP_SPARROW: ${BOOSTSTRAP_SPARROW}")

# Where to output the results of the compilation
SET(OutDir ${CMAKE_CURRENT_SOURCE_DIR}/.out)
SET(EXECUTABLE_OUTPUT_PATH ${OutDir})
SET(LIBRARY_OUTPUT_PATH ${OutDir})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OutDir})
SET(RUNTIME_OUTPUT_DIRECTORY ${OutDir})
message(STATUS "OutDir: ${OutDir}")
message(STATUS "exe output path: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "lib output path: ${LIBRARY_OUTPUT_PATH}")

# Conan dependency
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Find the boost library
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
SET(BOOST_LIBRARYDIR "" CACHE PATH "Path to Boost libraries")
find_package( Boost 1.60.0 COMPONENTS filesystem system program_options timer REQUIRED )
message(STATUS "Boost include: ${Boost_INCLUDE_DIRS}")
include_directories( "${Boost_INCLUDE_DIRS}" )
link_directories(${Boost_LIBRARY_DIRS})

# Find the LLVM library
set(LLVM_DIR ${CONAN_LLVM_ROOT}/share/llvm/cmake/)
find_package( LLVM 3.5.2 REQUIRED )
include_directories( "${LLVM_INCLUDE_DIRS}" )
link_directories(${LLVM_LIBRARY_DIRS})
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")

# Project options
if( NOT CMAKE_BUILD_TYPE )      # set default cmake build type to RelWithDebInfo (None Debug Release RelWithDebInfo MinSizeRel)
    set( CMAKE_BUILD_TYPE "RelWithDebInfo" )
endif()

# Compilation flags
if(MSVC)
    add_definitions( "-W4" )        # Warning level 4
    add_definitions( -DWIN32 )
    add_definitions( -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE )
else()
    add_definitions( -D__STDC_LIMIT_MACROS=1 )
    set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=gnu++0x" )
    add_definitions( -Wall )                    # All warnings...
    # add_definitions( -Wno-c++11-extensions )    # ... except C++11 extensions warnings
    add_definitions( -Wno-deprecated )          # ... and except deprecated functions
    # add_definitions( -Wno-undefined-bool-conversion ) # ... and except undefined bool conversions
    # add_definitions( -Wno-typedef-redefinition )   # redefinition of typedef '...' is a C11 feature
    # add_definitions( -Wno-unused-local-typedef ) # unused typedef 'cons_element' (inside boost headers)
endif()

# Add our macros
include(Macros.cmake)

# Dive into subdirectories
add_subdirectory( src/Nest )
add_subdirectory( src/Feather )
add_subdirectory( src/LLVMBackend )
add_subdirectory( src/SparrowFrontend )
add_subdirectory( src/SparrowCompiler )
