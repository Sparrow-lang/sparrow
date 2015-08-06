#pragma once

#include <Nest/Intermediate/TypeRef.h>

namespace SprFrontend
{
    namespace StdDef
    {
        extern Nest::TypeRef typeType;
        extern Nest::TypeRef typeUninitialized;
        extern Nest::TypeRef typeVoid;
        extern Nest::TypeRef typeNull;
        extern Nest::TypeRef typeBool;
        extern Nest::TypeRef typeByte;
        extern Nest::TypeRef typeInt;
        extern Nest::TypeRef typeSizeType;
        extern Nest::TypeRef typeStringRef;

        extern Nest::TypeRef typeRefType;
        extern Nest::TypeRef typeRefByte;
        extern Nest::TypeRef typeRefInt;
        extern Nest::TypeRef typeSizeTypeCt;

        extern Node* clsType;
        extern Node* clsUninitialized;
        extern Node* clsBool;

        extern Node* opRefEq;
        extern Node* opRefNe;
    }

    /// Creates the Type class & corresponding type; we need it before loading anything else
    void initTypeType(Nest::CompilationContext* ctx);
    
    void checkStdClass(DynNode* cls);
    void checkStdFunction(DynNode* cls);
}
