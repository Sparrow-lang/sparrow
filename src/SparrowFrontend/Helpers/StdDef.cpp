#include <StdInc.h>
#include "StdDef.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

using namespace SprFrontend;
using namespace SprFrontend::StdDef;
using namespace Nest;

// TODO (refactoring): Remove the Void type

TypeRef StdDef::typeType = nullptr;
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
Node* StdDef::clsBool = nullptr;

Node* StdDef::opRefEq = nullptr;
Node* StdDef::opRefNe = nullptr;

namespace {
bool classesFound = false;
bool functionsFound = false;
} // namespace

void SprFrontend::initTypeType(CompilationContext* ctx) {
    if (typeType)
        return;

    clsType = Feather_mkClass(NOLOC, fromCStr("Type"), {});
    Feather_setShouldAddToSymTab(clsType, 0);
    Nest_setPropertyString(clsType, propNativeName, fromCStr("Type"));
    Feather_setEvalMode(clsType, modeCt);
    Nest_setContext(clsType, ctx);
    setAccessType(clsType, publicAccess);
    if (!Nest_computeType(clsType))
        REP_INTERNAL(NOLOC, "Cannot create 'Type' type");
    typeType = Feather_getDataType(clsType, 0, modeCt);
}

void SprFrontend::checkStdClass(Node* cls) {
    if (classesFound)
        return;

    ASSERT(cls->nodeKind == nkFeatherDeclClass);

    StringRef clsName = Feather_getName(cls);

    if (clsName == "Void")
        StdDef::typeVoid = Feather_getDataType(cls, 0, modeRt);
    else if (clsName == "Null")
        StdDef::typeNull = Feather_getDataType(cls, 0, modeRt);
    else if (clsName == "Bool") {
        StdDef::clsBool = cls;
        StdDef::typeBool = Feather_getDataType(cls, 0, modeRt);
    } else if (clsName == "Byte") {
        StdDef::typeByte = Feather_getDataType(cls, 0, modeRt);
        StdDef::typeRefByte = Feather_getDataType(cls, 1, modeRt);
    } else if (clsName == "Int") {
        StdDef::typeInt = Feather_getDataType(cls, 0, modeRt);
        StdDef::typeRefInt = Feather_getDataType(cls, 1, modeRt);
    } else if (clsName == "SizeType") {
        StdDef::typeSizeType = Feather_getDataType(cls, 0, modeRt);
        StdDef::typeSizeTypeCt = Feather_getDataType(cls, 0, modeCt);
    } else if (clsName == "Type") {
        StdDef::typeType = Feather_getDataType(cls, 0, modeRt);
        StdDef::typeRefType = Feather_getDataType(cls, 1, modeRt);
    } else if (clsName == "StringRef")
        StdDef::typeStringRef = Feather_getDataType(cls, 0, modeRt);

    classesFound = StdDef::typeVoid != nullptr && StdDef::typeNull != nullptr &&
                   StdDef::typeBool != nullptr && StdDef::typeByte != nullptr &&
                   StdDef::typeInt != nullptr && StdDef::typeSizeType != nullptr &&
                   StdDef::typeType != nullptr && StdDef::typeStringRef != nullptr &&
                   StdDef::typeRefByte != nullptr && StdDef::typeRefType != nullptr &&
                   StdDef::typeSizeTypeCt != nullptr;
}

void SprFrontend::checkStdFunction(Node* fun) {
    if (functionsFound)
        return;

    Node* f = resultingDecl(fun);
    ASSERT(f && f->nodeKind == nkFeatherDeclFunction);

    StringRef funName = Feather_getName(fun);
    if (funName == "implOpRefEQ")
        StdDef::opRefEq = f;
    if (funName == "implOpRefNE")
        StdDef::opRefNe = f;

    functionsFound = StdDef::opRefEq != nullptr && StdDef::opRefNe != nullptr;
}
