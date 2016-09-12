# Top-Level CMakeLists.txt

cmake_minimum_required(VERSION 2.8.11)

# Project information
project( SparrowCompiler )
set( ${PROJECT_NAME}_MAJOR_VERSION  0 )
set( ${PROJECT_NAME}_MINOR_VERSION  9 )
set( ${PROJECT_NAME}_PATCH_LEVEL    3 )

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

# Find flex & bison
set(Old_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_PREFIX_PATH ${CONAN_BISON_ROOT} ${CONAN_FLEX_ROOT} ${CMAKE_PREFIX_PATH})
find_package(BISON)
find_package(FLEX)
message(STATUS "Flex executable: ${FLEX_EXECUTABLE}")
message(STATUS "Flex include: ${FLEX_INCLUDE_DIR}")
set(CMAKE_PREFIX_PATH ${Old_CMAKE_PREFIX_PATH})

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
    add_definitions( -Wno-deprecated )
    # add_definitions( -Wno-unused-local-typedef ) # unused typedef 'cons_element' (inside boost headers)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()

# Add our macros
include(Macros.cmake)

# Dive into subdirectories
add_subdirectory( src/Nest )
add_subdirectory( src/Feather )
add_subdirectory( src/LLVMBackend )
add_subdirectory( src/SparrowFrontend )
add_subdirectory( src/SparrowCompiler )
