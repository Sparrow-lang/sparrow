
# The source files
SET(sourceFiles
    "Api/Feather.h"

    "Utils/FeatherUtils.h"
    "Utils/FeatherUtils.hpp"

    "src/CtApiFunctions.h"
    "src/CtApiFunctions.cpp"
    "src/FeatherNodeCommonsCpp.h"
    "src/FeatherNodeCommonsH.h"
    "src/StdInc.h"
    "src/Api/Feather.c"
    "src/Api/Feather_Nodes.cpp"
    "src/Api/Feather_Nodes.h"
    "src/Api/Feather_Types.cpp"
    "src/Api/Feather_Types.h"
    "src/Utils/FeatherUtils.c"
    "src/Utils/FeatherUtils.cpp"
)

# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
ADD_DEFINITIONS( "-DINSIDE_FEATHER" )
ADD_SOURCES_TO_GROUPS(sourceFiles)

ADD_LIBRARY( Feather ${sourceFiles} )
