#include <StdInc.h>
#include "StdDef.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace SprFrontend::StdDef;
using namespace Feather;
using namespace Nest;

// TODO (refactoring): Remove the Void type

TypeRef StdDef::typeType = nullptr;
TypeRef StdDef::typeUninitialized = nullptr;
TypeRef StdDef::typeVoid = nullptr;
TypeRef StdDef::typeNull = nullptr;
TypeRef StdDef::typeBool = nullptr;
TypeRef StdDef::typeByte = nullptr;
TypeRef StdDef::typeInt = nullptr;
TypeRef StdDef::typeSizeType = nullptr;
TypeRef StdDef::typeStringRef = nullptr;

TypeRef StdDef::typeRefType = nullptr;
TypeRef StdDef::typeRefByte = nullptr;
TypeRef StdDef::typeRefInt = nullptr;
TypeRef StdDef::typeSizeTypeCt = nullptr;

Node* StdDef::clsType = nullptr;
Node* StdDef::clsUninitialized = nullptr;
Node* StdDef::clsBool = nullptr;

Node* StdDef::opRefEq = nullptr;
Node* StdDef::opRefNe = nullptr;

namespace
{
    bool classesFound = false;
    bool functionsFound = false;
}

void SprFrontend::initTypeType(CompilationContext* ctx)
{
    if ( typeType )
        return;
    
    clsType = mkClass(NOLOC, "Type", {});
    setShouldAddToSymTab(clsType, false);
    setProperty(clsType, propNativeName, string("Type"));
    setEvalMode(clsType, modeCt);
    setContext(clsType, ctx);
    computeType(clsType);
    typeType = getDataType(clsType, 0, modeCt);
}

void SprFrontend::checkStdClass(Node* cls)
{
    if ( classesFound )
        return;

    ASSERT(cls->nodeKind == nkFeatherDeclClass);

    if ( getName(cls) == "Void" )
        StdDef::typeVoid = getDataType(cls);
    else if ( getName(cls) == "Null" )
        StdDef::typeNull = getDataType(cls);
    else if ( getName(cls) == "Bool" )
    {
        StdDef::clsBool = cls;
        StdDef::typeBool = getDataType(cls);
    }
    else if ( getName(cls) == "Byte" )
    {
        StdDef::typeByte = getDataType(cls);
        StdDef::typeRefByte = getDataType(cls, 1);
    }
    else if ( getName(cls) == "Int" )
    {
        StdDef::typeInt = getDataType(cls);
        StdDef::typeRefInt = getDataType(cls, 1);
    }
    else if ( getName(cls) == "SizeType" )
    {
        StdDef::typeSizeType = getDataType(cls);
        StdDef::typeSizeTypeCt = getDataType(cls, 0, modeCt);
    }
    else if ( getName(cls) == "Uninitialized" )
    {
        StdDef::clsUninitialized = cls;
        StdDef::typeUninitialized = getDataType(cls);
    }
    else if ( getName(cls) == "Type" )
    {
        StdDef::typeType = getDataType(cls);
        StdDef::typeRefType = getDataType(cls, 1);
    }
    else if ( getName(cls) == "StringRef" )
        StdDef::typeStringRef = getDataType(cls);

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

    Node* f = resultingDecl(fun);
    ASSERT(f && f->nodeKind == nkFeatherDeclFunction);

    if ( getName(fun) == "_opRefEQ" )
        StdDef::opRefEq = f;
    if ( getName(fun) == "_opRefNE" )
        StdDef::opRefNe = f;

    functionsFound = StdDef::opRefEq != nullptr && StdDef::opRefNe != nullptr;
}
