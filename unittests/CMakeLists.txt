
# The source files
SET(sourceFiles
    "StdInc.h"
    "UnitTestsMain.cpp"
    "DetailConsoleReporter.cpp"
    "Common/BackendMock.cpp"
    "Common/BackendMock.hpp"
    "Common/FeatherNodeFactory.cpp"
    "Common/FeatherNodeFactory.hpp"
    "Common/GeneralFixture.cpp"
    "Common/GeneralFixture.hpp"
    "Common/RcBasic.hpp"
    "Common/TypeFactory.cpp"
    "Common/TypeFactory.hpp"
    "Nest/MockNode.cpp"
    "Nest/MockNode.hpp"
    "Nest/MockType.cpp"
    "Nest/MockType.hpp"
    "Nest/TestNode.cpp"
    "Nest/TestType.cpp"
    "Feather/TestFeatherNodes.cpp"
    "Feather/TestTypes.cpp"
    "SparrowFrontend/metatests/TestGenCallableDecl.cpp"
    "SparrowFrontend/metatests/TestGenGenericParams.cpp"
    "SparrowFrontend/metatests/TestGenValueForType.cpp"
    "SparrowFrontend/SprCommon/ConceptsServiceMock.cpp"
    "SparrowFrontend/SprCommon/GenCallableDecl.cpp"
    "SparrowFrontend/SprCommon/GenGenericParams.cpp"
    "SparrowFrontend/SprCommon/GenValueForType.cpp"
    "SparrowFrontend/SprCommon/OverloadServiceMock.cpp"
    "SparrowFrontend/SprCommon/SampleTypes.cpp"
    "SparrowFrontend/TestCallable.cpp"
    "SparrowFrontend/TestConvert.cpp"
    "SparrowFrontend/TestDecls.cpp"
    "SparrowFrontend/TestGenerics.cpp"
)

# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( "../src" )
INCLUDE_DIRECTORIES( "../externals/rapidcheck/include" )
INCLUDE_DIRECTORIES( "../externals/rapidcheck/ext/catch/include" )
INCLUDE_DIRECTORIES( "../externals/rapidcheck/extras/catch/include" )
ADD_SOURCES_TO_GROUPS(sourceFiles)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})
ADD_EXECUTABLE( SparrowUnitTests ${sourceFiles} )
TARGET_LINK_LIBRARIES(SparrowUnitTests SparrowFrontend LLVMBackend Feather Nest)
TARGET_LINK_LIBRARIES(SparrowUnitTests boost_timer boost_chrono rapidcheck)
TARGET_LINK_LIBRARIES(SparrowUnitTests c++fs)
