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

void SprFrontend::checkStdClass(DynNode* cls)
{
    if ( classesFound )
        return;

    ASSERT(cls->nodeKind() == nkFeatherDeclClass);
    Node* c = cls->node();

    if ( getName(c) == "Void" )
        StdDef::typeVoid = getDataType(c);
    else if ( getName(c) == "Null" )
        StdDef::typeNull = getDataType(c);
    else if ( getName(c) == "Bool" )
    {
        StdDef::clsBool = c;
        StdDef::typeBool = getDataType(c);
    }
    else if ( getName(c) == "Byte" )
    {
        StdDef::typeByte = getDataType(c);
        StdDef::typeRefByte = getDataType(c, 1);
    }
    else if ( getName(c) == "Int" )
    {
        StdDef::typeInt = getDataType(c);
        StdDef::typeRefInt = getDataType(c, 1);
    }
    else if ( getName(c) == "SizeType" )
    {
        StdDef::typeSizeType = getDataType(c);
        StdDef::typeSizeTypeCt = getDataType(c, 0, modeCt);
    }
    else if ( getName(c) == "Uninitialized" )
    {
        StdDef::clsUninitialized = c;
        StdDef::typeUninitialized = getDataType(c);
    }
    else if ( getName(c) == "Type" )
    {
        StdDef::typeType = getDataType(c);
        StdDef::typeRefType = getDataType(c, 1);
    }
    else if ( getName(c) == "StringRef" )
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

    Node* f = resultingDecl(fun->node());
    ASSERT(f && f->nodeKind == nkFeatherDeclFunction);

    if ( getName(fun->node()) == "_opRefEQ" )
        StdDef::opRefEq = f;
    if ( getName(fun->node()) == "_opRefNE" )
        StdDef::opRefNe = f;

    functionsFound = StdDef::opRefEq != nullptr && StdDef::opRefNe != nullptr;
}
