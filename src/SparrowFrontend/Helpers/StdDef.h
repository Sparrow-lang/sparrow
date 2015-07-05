#pragma once

#include <Nest/Intermediate/TypeRef.h>

FWD_CLASS1(Feather, Class);
FWD_CLASS1(Feather, Function);

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

        extern Feather::Class* clsType;
        extern Feather::Class* clsUninitialized;
        extern Feather::Class* clsBool;

        extern Feather::Function* opRefEq;
        extern Feather::Function* opRefNe;
    }

    /// Creates the Type class & corresponding type; we need it before loading anything else
    void initTypeType(Nest::CompilationContext* ctx);
    
    void checkStdClass(Nest::Node* cls);
    void checkStdFunction(Nest::Node* cls);
}
