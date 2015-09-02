
# The source files
SET(sourceFiles
    "StdInc.h"
    "StdInc.cpp"
    "DefaultStdInc.h"
    "Nest.h"
    "Nest.cpp"
    "CompilerModule.h"
    "Compiler.h"
    "CompilerImpl.h"
    "CompilerImpl.cpp"
    "CompilerInstance.h"
    "CompilerInstance.cpp"
    "CompilerSettings.h"
    "Common/Fwd.h"
    "Common/Alloc.h"
    "Common/Alloc.c"
    "Common/Assert.h"
    "Common/Tuple.h"
    "Common/StringRef.h"
    "Common/SaveValue.h"
    "Common/ScopeGuard.h"
    "Common/ScopeGuard.cpp"
    "Common/Diagnostic.h"
    "Common/Diagnostic.hpp"
    "Common/Diagnostic.cpp"
    "Common/ConsoleColors.h"
    "Common/DiagnosticFormatter.h"
    "Common/DiagnosticFormatter.cpp"
    "Common/PrintTimer.h"
    "Intermediate/NodeVector.h"
    "Intermediate/Node.h"
    "Intermediate/Node.cpp"
    "Intermediate/NodeRange.h"
    "Intermediate/NodeRange.c"
    "Intermediate/NodeArray.h"
    "Intermediate/NodeArray.c"
    "Intermediate/NodeKindRegistrar.h"
    "Intermediate/NodeKindRegistrar.cpp"
    "Intermediate/Modifier.h"
    "Intermediate/EvalMode.h"
    "Intermediate/SymTab.h"
    "Intermediate/SymTab.cpp"
    "Intermediate/CompilationContext.h"
    "Intermediate/CompilationContext.cpp"
    "Intermediate/TypeRef.h"
    "Intermediate/Type.h"
    "Intermediate/Type.cpp"
    "Intermediate/TypeKindRegistrar.h"
    "Intermediate/TypeKindRegistrar.cpp"
    "Frontend/SourceCodeKindRegistrar.h"
    "Frontend/SourceCodeKindRegistrar.cpp"
    "Frontend/SourceCode.h"
    "Frontend/SourceCode.cpp"
    "Frontend/Location.h"
    "Frontend/Location.c"
    "Backend/Backend.h"
    "Backend/Backend.cpp"
)

# Project settings
INCLUDE_DIRECTORIES( "." )
INCLUDE_DIRECTORIES( ".." )
ADD_DEFINITIONS( "-DINSIDE_NEST" )
ADD_MSVC_PRECOMPILED_HEADER("StdInc.h" "StdInc.cpp" sourceFiles)
ADD_SOURCES_TO_GROUPS(sourceFiles)

ADD_LIBRARY( Nest ${sourceFiles} )
