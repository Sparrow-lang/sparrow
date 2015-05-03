#pragma once

FWD_CLASS1(Feather, Class);
FWD_CLASS1(Feather, Function);
FWD_CLASS1(Nest, Type);

namespace SprFrontend
{
    namespace StdDef
    {
        extern Nest::Type* typeType;
        extern Nest::Type* typeUninitialized;
        extern Nest::Type* typeVoid;
        extern Nest::Type* typeNull;
        extern Nest::Type* typeBool;
        extern Nest::Type* typeByte;
        extern Nest::Type* typeInt;
        extern Nest::Type* typeSizeType;
        extern Nest::Type* typeStringRef;

        extern Nest::Type* typeRefType;
        extern Nest::Type* typeRefByte;
        extern Nest::Type* typeRefInt;
        extern Nest::Type* typeSizeTypeCt;

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
