
# The source files
SET(sourceFiles
    "StdInc.h"
    "StdInc.cpp"
    "FeatherNodeCommonsH.h"
    "FeatherNodeCommonsCpp.h"
    "FeatherTypes.h"
    "FeatherTypes.cpp"
    "CtApiFunctions.h"
    "CtApiFunctions.cpp"
    "Nodes/Properties.h"
    "Nodes/Properties.cpp"
    "Nodes/FeatherNodes.h"
    "Nodes/FeatherNodes.cpp"
    "Nodes/Exp/AtomicOrdering.h"
    "Nodes/Decls/CallConvention.h"
    "Util/Context.h"
    "Util/Context.cpp"
    "Util/Decl.h"
    "Util/Decl.cpp"
    "Util/Ct.h"
    "Util/Ct.cpp"
    "Util/TypeTraits.h"
    "Util/TypeTraits.cpp"
    "Util/StringData.h"
    "Frontend/FSimpleSourceCode.h"
    "Frontend/FSimpleSourceCode.cpp"
    "Frontend/Impl/SimpleTokenType.h"
    "Frontend/Impl/SimpleToken.h"
    "Frontend/Impl/SimpleLexer.h"
    "Frontend/Impl/SimpleLexer.cpp"
    "Frontend/Impl/SimpleAstNode.h"
    "Frontend/Impl/SimpleAstNode.cpp"
    "Frontend/Impl/SimpleParser.h"
    "Frontend/Impl/SimpleParser.cpp"
)

# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
ADD_DEFINITIONS( "-DINSIDE_FEATHER" )
ADD_MSVC_PRECOMPILED_HEADER("StdInc.h" "StdInc.cpp" sourceFiles)
ADD_SOURCES_TO_GROUPS(sourceFiles)

ADD_LIBRARY( Feather ${sourceFiles} )
