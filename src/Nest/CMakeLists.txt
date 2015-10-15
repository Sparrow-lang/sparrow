
# The source files
SET(sourceFiles
    "Api/Backend.cpp"
    "Api/Backend.cpp"
    "Api/Backend.h"
    "Api/Backend.h"
    "Api/CompilationContext.cpp"
    "Api/CompilationContext.h"
    "Api/Compiler.cpp"
    "Api/Compiler.h"
    "Api/CompilerModule.h"
    "Api/EvalMode.h"
    "Api/Location.c"
    "Api/Location.h"
    "Api/Modifier.h"
    "Api/Nest.cpp"
    "Api/Nest.h"
    "Api/Node.cpp"
    "Api/Node.h"
    "Api/NodeArray.c"
    "Api/NodeArray.h"
    "Api/NodeKindRegistrar.cpp"
    "Api/NodeKindRegistrar.h"
    "Api/NodeProperties.c"
    "Api/NodeProperties.h"
    "Api/NodeRange.c"
    "Api/NodeRange.h"
    "Api/SourceCode.cpp"
    "Api/SourceCode.h"
    "Api/SourceCodeKindRegistrar.cpp"
    "Api/SourceCodeKindRegistrar.h"
    "Api/SymTab.cpp"
    "Api/SymTab.h"
    "Api/Type.cpp"
    "Api/Type.h"
    "Api/TypeKindRegistrar.cpp"
    "Api/TypeKindRegistrar.h"
    "Api/TypeRef.h"

    "Utils/Alloc.c"
    "Utils/Alloc.h"
    "Utils/Assert.h"
    "Utils/CompilerSettings.hpp"
    "Utils/ConsoleColors.hpp"
    "Utils/DefaultStdInc.hpp"
    "Utils/Diagnostic.cpp"
    "Utils/Diagnostic.h"
    "Utils/Diagnostic.hpp"
    "Utils/DiagnosticFormatter.cpp"
    "Utils/DiagnosticFormatter.hpp"
    "Utils/Fwd.hpp"
    "Utils/NodeUtils.hpp"
    "Utils/NodeVector.hpp"
    "Utils/PrintTimer.hpp"
    "Utils/PtrArray.c"
    "Utils/PtrArray.h"
    "Utils/PtrRange.h"
    "Utils/StringRef.cpp"
    "Utils/StringRef.h"
    "Utils/StringRef.hpp"
    "Utils/Tuple.hpp"

    "Internal/StdInc.cpp"
    "Internal/StdInc.hpp"
)

# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
ADD_DEFINITIONS( "-DINSIDE_NEST" )
ADD_MSVC_PRECOMPILED_HEADER("StdInc.h" "StdInc.cpp" sourceFiles)
ADD_SOURCES_TO_GROUPS(sourceFiles)

ADD_LIBRARY( Nest ${sourceFiles} )
