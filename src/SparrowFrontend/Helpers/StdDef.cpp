#include <StdInc.h>
#include "StdDef.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Type/DataType.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace SprFrontend::StdDef;
using namespace Feather;
using namespace Nest;

// TODO (refactoring): Remove the Void type

Nest::Type* StdDef::typeType = nullptr;
Nest::Type* StdDef::typeUninitialized = nullptr;
Nest::Type* StdDef::typeVoid = nullptr;
Nest::Type* StdDef::typeNull = nullptr;
Nest::Type* StdDef::typeBool = nullptr;
Nest::Type* StdDef::typeByte = nullptr;
Nest::Type* StdDef::typeInt = nullptr;
Nest::Type* StdDef::typeSizeType = nullptr;
Nest::Type* StdDef::typeStringRef = nullptr;

Nest::Type* StdDef::typeRefType = nullptr;
Nest::Type* StdDef::typeRefByte = nullptr;
Nest::Type* StdDef::typeRefInt = nullptr;
Nest::Type* StdDef::typeSizeTypeCt = nullptr;

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
    typeType = DataType::get(clsType, 0, modeCt);
}

void SprFrontend::checkStdClass(Node* cls)
{
    if ( classesFound )
        return;

    ASSERT(cls->nodeKind() == nkFeatherDeclClass);
    Class* c = (Class*) cls;

    if ( getName(cls) == "Void" )
        StdDef::typeVoid = DataType::get(c);
    else if ( getName(cls) == "Null" )
        StdDef::typeNull = DataType::get(c);
    else if ( getName(cls) == "Bool" )
    {
        StdDef::clsBool = c;
        StdDef::typeBool = DataType::get(c);
    }
    else if ( getName(cls) == "Byte" )
    {
        StdDef::typeByte = DataType::get(c);
        StdDef::typeRefByte = DataType::get(c, 1);
    }
    else if ( getName(cls) == "Int" )
    {
        StdDef::typeInt = DataType::get(c);
        StdDef::typeRefInt = DataType::get(c, 1);
    }
    else if ( getName(cls) == "SizeType" )
    {
        StdDef::typeSizeType = DataType::get(c);
        StdDef::typeSizeTypeCt = DataType::get(c, 0, modeCt);
    }
    else if ( getName(cls) == "Uninitialized" )
    {
        StdDef::clsUninitialized = c;
        StdDef::typeUninitialized = DataType::get(c);
    }
    else if ( getName(cls) == "Type" )
    {
        StdDef::typeType = DataType::get(c);
        StdDef::typeRefType = DataType::get(c, 1);
    }
    else if ( getName(cls) == "StringRef" )
        StdDef::typeStringRef = DataType::get(c);

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

void SprFrontend::checkStdFunction(Node* fun)
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
