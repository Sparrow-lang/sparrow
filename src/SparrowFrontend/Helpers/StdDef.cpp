#include <StdInc.h>
#include "StdDef.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace SprFrontend::StdDef;
using namespace Feather;
using namespace Nest;

// TODO (refactoring): Remove the Void type

Nest::TypeRef StdDef::typeType = nullptr;
Nest::TypeRef StdDef::typeUninitialized = nullptr;
Nest::TypeRef StdDef::typeVoid = nullptr;
Nest::TypeRef StdDef::typeNull = nullptr;
Nest::TypeRef StdDef::typeBool = nullptr;
Nest::TypeRef StdDef::typeByte = nullptr;
Nest::TypeRef StdDef::typeInt = nullptr;
Nest::TypeRef StdDef::typeSizeType = nullptr;
Nest::TypeRef StdDef::typeStringRef = nullptr;

Nest::TypeRef StdDef::typeRefType = nullptr;
Nest::TypeRef StdDef::typeRefByte = nullptr;
Nest::TypeRef StdDef::typeRefInt = nullptr;
Nest::TypeRef StdDef::typeSizeTypeCt = nullptr;

Feather::Class* StdDef::clsType = nullptr;
Feather::Class* StdDef::clsUninitialized = nullptr;
Feather::Class* StdDef::clsBool = nullptr;

Feather::Function* StdDef::opRefEq = nullptr;
Feather::Function* StdDef::opRefNe = nullptr;

namespace
{
    bool classesFound = false;
    bool functionsFound = false;
}

void SprFrontend::initTypeType(CompilationContext* ctx)
{
    if ( typeType )
        return;
    
    clsType = (Class*) mkClass(NOLOC, "Type", {});
    setShouldAddToSymTab(clsType, false);
    clsType->setProperty(propNativeName, string("Type"));
    setEvalMode(clsType, modeCt);
    clsType->setContext(ctx);
    clsType->computeType();
    typeType = getDataType(clsType, 0, modeCt);
}

void SprFrontend::checkStdClass(DynNode* cls)
{
    if ( classesFound )
        return;

    ASSERT(cls->nodeKind() == nkFeatherDeclClass);
    Class* c = (Class*) cls;

    if ( getName(cls) == "Void" )
        StdDef::typeVoid = getDataType(c);
    else if ( getName(cls) == "Null" )
        StdDef::typeNull = getDataType(c);
    else if ( getName(cls) == "Bool" )
    {
        StdDef::clsBool = c;
        StdDef::typeBool = getDataType(c);
    }
    else if ( getName(cls) == "Byte" )
    {
        StdDef::typeByte = getDataType(c);
        StdDef::typeRefByte = getDataType(c, 1);
    }
    else if ( getName(cls) == "Int" )
    {
        StdDef::typeInt = getDataType(c);
        StdDef::typeRefInt = getDataType(c, 1);
    }
    else if ( getName(cls) == "SizeType" )
    {
        StdDef::typeSizeType = getDataType(c);
        StdDef::typeSizeTypeCt = getDataType(c, 0, modeCt);
    }
    else if ( getName(cls) == "Uninitialized" )
    {
        StdDef::clsUninitialized = c;
        StdDef::typeUninitialized = getDataType(c);
    }
    else if ( getName(cls) == "Type" )
    {
        StdDef::typeType = getDataType(c);
        StdDef::typeRefType = getDataType(c, 1);
    }
    else if ( getName(cls) == "StringRef" )
        StdDef::typeStringRef = getDataType(c);

    classesFound = StdDef::typeVoid != nullptr
                && StdDef::typeNull != nullptr
                && StdDef::typeBool != nullptr
                && StdDef::typeByte != nullptr
                && StdDef::typeInt != nullptr
                && StdDef::typeSizeType != nullptr
                && StdDef::typeType != nullptr
                && StdDef::typeStringRef != nullptr
                && StdDef::typeRefByte != nullptr
                && StdDef::typeRefType != nullptr
                && StdDef::typeSizeTypeCt != nullptr;
}

void SprFrontend::checkStdFunction(DynNode* fun)
{
    if ( functionsFound )
        return;

    Function* f = resultingDecl(fun)->as<Function>();
    ASSERT(f);

    if ( getName(fun) == "_opRefEQ" )
        StdDef::opRefEq = f;
    if ( getName(fun) == "_opRefNE" )
        StdDef::opRefNe = f;

    functionsFound = StdDef::opRefEq != nullptr && StdDef::opRefNe != nullptr;
}
