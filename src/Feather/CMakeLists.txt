
# The source files
SET(sourceFiles
    "Api/Feather.h"

    "Utils/Context.h"
    "Utils/Ct.h"
    "Utils/Decl.h"
    "Utils/Properties.h"
    "Utils/StringData.h"
    "Utils/TypeTraits.h"
    "Utils/NodeUtils.h"

    "src/CtApiFunctions.h"
    "src/CtApiFunctions.cpp"
    "src/FeatherNodeCommonsCpp.h"
    "src/FeatherNodeCommonsH.h"
    "src/StdInc.cpp"
    "src/StdInc.h"
    "src/Api/Feather.c"
    "src/Api/Feather_Nodes.cpp"
    "src/Api/Feather_Nodes.h"
    "src/Api/Feather_Types.cpp"
    "src/Api/Feather_Types.h"
    "src/Utils/Context.cpp"
    "src/Utils/Ct.cpp"
    "src/Utils/Decl.cpp"
    "src/Utils/Properties.cpp"
    "src/Utils/TypeTraits.cpp"
)

# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
ADD_DEFINITIONS( "-DINSIDE_FEATHER" )
ADD_MSVC_PRECOMPILED_HEADER("StdInc.h" "StdInc.cpp" sourceFiles)
ADD_SOURCES_TO_GROUPS(sourceFiles)

ADD_LIBRARY( Feather ${sourceFiles} )
