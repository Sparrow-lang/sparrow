
# The source files
SET(sourceFiles
    "StdInc.h"
    "UnitTestsMain.cpp"
    "TestBasic.cpp"
    "TestConvert.cpp"
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
