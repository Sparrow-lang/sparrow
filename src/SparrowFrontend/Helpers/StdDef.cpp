#include <StdInc.h>
#include "StdDef.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

using namespace SprFrontend;
using namespace SprFrontend::StdDef;
using namespace Nest;

// TODO (refactoring): Remove the Void type

TypeWithStorage StdDef::typeType = nullptr;
Type StdDef::typeVoid = nullptr;
TypeWithStorage StdDef::typeNull = nullptr;
TypeWithStorage StdDef::typeBool = nullptr;
TypeWithStorage StdDef::typeByte = nullptr;
TypeWithStorage StdDef::typeInt = nullptr;
TypeWithStorage StdDef::typeStringRef = nullptr;

TypeWithStorage StdDef::typeRefType = nullptr;
TypeWithStorage StdDef::typeRefByte = nullptr;
TypeWithStorage StdDef::typeRefInt = nullptr;

Node* StdDef::clsType = nullptr;
Node* StdDef::clsNull = nullptr;
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

    clsType = Feather_mkClass(NOLOC, StringRef("Type"), {});
    Feather_setShouldAddToSymTab(clsType, 0);
    Nest_setPropertyString(clsType, propNativeName, StringRef("Type"));
    Feather_setEvalMode(clsType, modeCt);
    Nest_setContext(clsType, ctx);
    setAccessType(clsType, publicAccess);
    if (!Nest_computeType(clsType))
        REP_INTERNAL(NOLOC, "Cannot create 'Type' type");
    typeType = Feather::DataType::get(clsType, modeCt);
}

void SprFrontend::checkStdClass(Node* cls) {
    if (classesFound)
        return;

    ASSERT(cls->nodeKind == nkFeatherDeclClass);

    StringRef clsName = Feather_getName(cls);

    if (clsName == "Void")
        StdDef::typeVoid = Feather::getDataTypeWithPtr(cls, 0, modeRt);
    else if (clsName == "Null") {
        StdDef::clsNull = cls;
        StdDef::typeNull = Feather::getDataTypeWithPtr(cls, 0, modeRt);
    } else if (clsName == "Bool") {
        StdDef::clsBool = cls;
        StdDef::typeBool = Feather::getDataTypeWithPtr(cls, 0, modeRt);
    } else if (clsName == "Int8") {
        StdDef::typeByte = Feather::getDataTypeWithPtr(cls, 0, modeRt);
        StdDef::typeRefByte = Feather::getDataTypeWithPtr(cls, 1, modeRt);
    } else if (clsName == "Int") {
        StdDef::typeInt = Feather::getDataTypeWithPtr(cls, 0, modeRt);
        StdDef::typeRefInt = Feather::getDataTypeWithPtr(cls, 1, modeRt);
    } else if (clsName == "Type") {
        StdDef::typeType = Feather::getDataTypeWithPtr(cls, 0, modeRt);
        StdDef::typeRefType = Feather::getDataTypeWithPtr(cls, 1, modeRt);
    } else if (clsName == "StringRef")
        StdDef::typeStringRef = Feather::getDataTypeWithPtr(cls, 0, modeRt);

    classesFound = StdDef::typeVoid != nullptr && StdDef::typeNull != nullptr &&
                   StdDef::typeBool != nullptr && StdDef::typeByte != nullptr &&
                   StdDef::typeInt != nullptr && StdDef::typeType != nullptr &&
                   StdDef::typeStringRef != nullptr && StdDef::typeRefByte != nullptr &&
                   StdDef::typeRefType != nullptr;
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
