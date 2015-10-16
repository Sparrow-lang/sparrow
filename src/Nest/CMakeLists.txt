
# The source files
SET(sourceFiles
    "Api/Backend.h"
    "Api/CompilationContext.h"
    "Api/Compiler.h"
    "Api/CompilerModule.h"
    "Api/EvalMode.h"
    "Api/Location.h"
    "Api/Modifier.h"
    "Api/Nest.h"
    "Api/Node.h"
    "Api/NodeArray.h"
    "Api/NodeKindRegistrar.h"
    "Api/NodeProperties.h"
    "Api/NodeRange.h"
    "Api/SourceCode.h"
    "Api/SourceCodeKindRegistrar.h"
    "Api/StringRef.h"
    "Api/SymTab.h"
    "Api/Type.h"
    "Api/TypeKindRegistrar.h"
    "Api/TypeRef.h"

    "Utils/Alloc.h"
    "Utils/Assert.h"
    "Utils/CompilerSettings.hpp"
    "Utils/ConsoleColors.hpp"
    "Utils/DefaultStdInc.hpp"
    "Utils/Diagnostic.h"
    "Utils/Diagnostic.hpp"
    "Utils/DiagnosticFormatter.hpp"
    "Utils/Fwd.hpp"
    "Utils/LocationUtils.h"
    "Utils/NodeUtils.h"
    "Utils/NodeUtils.hpp"
    "Utils/NodeVector.hpp"
    "Utils/PrintTimer.hpp"
    "Utils/PtrArray.h"
    "Utils/PtrRange.h"
    "Utils/StringRef.hpp"
    "Utils/Tuple.hpp"

    "src/StdInc.cpp"
    "src/StdInc.hpp"
    "src/Api/Backend.c"
    "src/Api/CompilationContext.c"
    "src/Api/Compiler.cpp"
    "src/Api/Nest.c"
    "src/Api/Node.c"
    "src/Api/NodeKindRegistrar.c"
    "src/Api/NodeProperties.c"
    "src/Api/SourceCode.c"
    "src/Api/SourceCodeKindRegistrar.c"
    "src/Api/SymTab.cpp"
    "src/Api/Type.cpp"
    "src/Api/TypeKindRegistrar.c"
    "src/Utils/Alloc.c"
    "src/Utils/Diagnostic.cpp"
    "src/Utils/DiagnosticFormatter.cpp"
    "src/Utils/LocationUtils.c"
    "src/Utils/NodeUtils.cpp"
    "src/Utils/PtrArray.c"
    "src/Utils/StringRef.cpp"
)

# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
ADD_DEFINITIONS( "-DINSIDE_NEST" )
ADD_MSVC_PRECOMPILED_HEADER("StdInc.h" "StdInc.cpp" sourceFiles)
ADD_SOURCES_TO_GROUPS(sourceFiles)

ADD_LIBRARY( Nest ${sourceFiles} )
