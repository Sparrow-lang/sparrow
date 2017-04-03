#pragma once

#include "Nest/Api/TypeRef.h"

namespace SprFrontend
{
    namespace StdDef
    {
        extern TypeRef typeType;
        extern TypeRef typeVoid;
        extern TypeRef typeNull;
        extern TypeRef typeBool;
        extern TypeRef typeByte;
        extern TypeRef typeInt;
        extern TypeRef typeSizeType;
        extern TypeRef typeStringRef;

        extern TypeRef typeRefType;
        extern TypeRef typeRefByte;
        extern TypeRef typeRefInt;
        extern TypeRef typeSizeTypeCt;

        extern Node* clsType;
        extern Node* clsBool;

        extern Node* opRefEq;
        extern Node* opRefNe;
    }

    /// Creates the Type class & corresponding type; we need it before loading anything else
    void initTypeType(CompilationContext* ctx);

    void checkStdClass(Node* cls);
    void checkStdFunction(Node* cls);
}
