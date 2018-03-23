# Top-Level CMakeLists.txt

cmake_minimum_required(VERSION 3.9.0)

# Project information
project( SparrowCompiler )

# Get compiler version
set(SparrowCompiler_BUILD_VERSION 0)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
    find_package(Git)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --long --tags --dirty --always
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE "SparrowCompiler_BUILD_VERSION"
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
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
option(BOOTSTRAP_SPARROW "Use system-wide SparrowCompiler to compile Sparrow files needed for the compiler" OFF)
message(STATUS "BOOTSTRAP_SPARROW: ${BOOTSTRAP_SPARROW}")

# Where to output the results of the compilation
set(OutDir ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)
set(EXECUTABLE_OUTPUT_PATH ${OutDir})
set(LIBRARY_OUTPUT_PATH ${OutDir})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OutDir})
set(RUNTIME_OUTPUT_DIRECTORY ${OutDir})
message(STATUS "OutDir: ${OutDir}")
message(STATUS "exe output path: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "lib output path: ${LIBRARY_OUTPUT_PATH}")

# Find the boost library
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(BOOST_LIBRARYDIR "" CACHE PATH "Path to Boost libraries")
find_package( Boost 1.54.0 COMPONENTS filesystem system program_options timer REQUIRED )
message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost lib dir: ${Boost_LIBRARY_DIRS}")
include_directories( "${Boost_INCLUDE_DIRS}" )
link_directories(${Boost_LIBRARY_DIRS})

# Find the LLVM library
find_package( LLVM 5.0.0 REQUIRED )
include_directories( "${LLVM_INCLUDE_DIRS}" )
link_directories(${LLVM_LIBRARY_DIRS})
message(STATUS "LLVM bin dir: ${LLVM_TOOLS_BINARY_DIR}")
message(STATUS "LLVM include dir: ${LLVM_INCLUDE_DIRS}")


# Project options
if( NOT CMAKE_BUILD_TYPE )      # set default cmake build type to Release (None Debug Release RelWithDebInfo MinSizeRel)
    set( CMAKE_BUILD_TYPE "Release" )
endif()

# Compilation flags
if(MSVC)
    add_definitions( "-W4" )        # Warning level 4
    add_definitions( -DWIN32 )
    add_definitions( -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE )
else()
    add_definitions( -D__STDC_LIMIT_MACROS=1 )
    set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11" )
    # set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address" )
    add_definitions( -Wall )                    # All warnings...
    add_definitions( -Wno-deprecated )          # ... and except deprecated functions
endif()

# Add our macros
include(Macros.cmake)

# Add target for clang-format
find_program(CLANG_FORMAT "clang-format" HINTS ${LLVM_TOOLS_BINARY_DIR})
if(CLANG_FORMAT)
    add_custom_target(
        clang-format
        COMMAND
            find ../src/ -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp'
            | xargs ${CLANG_FORMAT} -i
    )
endif()

# Add target for clang-tidy
set(TIDY_CHECKS
    #-*,   # Disable for non-debug builds

    modernize-*,
    cppcoreguidelines-*,
    -cppcoreguidelines-pro-bounds-array-to-pointer-decay, # not met, requires gsl
    -cppcoreguidelines-pro-bounds-constant-array-index, # not met, requires gsl
    -cppcoreguidelines-pro-bounds-pointer-arithmetic, # not met, requires gsl
    -cppcoreguidelines-pro-type-union-access, # not met (we use some unions)
    -cppcoreguidelines-pro-type-vararg, # not met (we are using printf)
)

find_program(CLANG_TIDY_SCRIPT "run-clang-tidy.py" HINTS ${LLVM_TOOLS_BINARY_DIR}/../share/clang)
if(CLANG_TIDY_SCRIPT)
    # Search for clang-tidy and clang-apply-replacements
    find_program(CLANG_TIDY "clang-tidy" HINTS ${LLVM_TOOLS_BINARY_DIR})
    find_program(CLANG_APPLY_REPLACEMENTS "clang-apply-replacements" HINTS ${LLVM_TOOLS_BINARY_DIR})

    # Set extra arguments
    set(TIDY_FIX 0)
    set(TIDY_EXTRA_ARGS -quiet -format -style file)
    if (TIDY_FIX)
        set(TIDY_EXTRA_ARGS ${TIDY_EXTRA_ARGS} -j 1 -fix)
    endif()

    # Actually add the target
    add_custom_target(
        clang-tidy
        COMMAND
            ${CLANG_TIDY_SCRIPT} -header-filter='.*\.hpp' -checks='${TIDY_CHECKS}'
            -clang-tidy-binary ${CLANG_TIDY}
            -clang-apply-replacements-binary ${CLANG_APPLY_REPLACEMENTS}
            ${TIDY_EXTRA_ARGS}
    )
endif()

# Ensure generation of compile_commands.json; needed for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dive into subdirectories
add_subdirectory( src/Nest )
add_subdirectory( src/Feather )
add_subdirectory( src/LLVMBackend )
add_subdirectory( src/SparrowFrontend )
add_subdirectory( src/SparrowCompiler )
