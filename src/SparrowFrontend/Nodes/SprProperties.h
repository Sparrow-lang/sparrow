#pragma once

#include "Feather/Utils/FeatherUtils.h"

namespace SprFrontend {
extern const char* propConvert;
extern const char* propTempVarContstruction;
extern const char* propNoDefault;
extern const char* propCtGeneric;
extern const char* propGenerateInitCtor;
extern const char* propMacro;
extern const char* propAllowDeclExp;
extern const char* propNoWarnIfNoDeclFound;
extern const char* propThisParamIdx;
extern const char* propThisParamType;
extern const char* propOverloadPrio;

constexpr const char* propVarInit = "spr.varInit";
constexpr const char* propSprGivenType = "spr.givenType";
constexpr const char* propSprLiteralType = "spr.literalType";
constexpr const char* propSprLiteralData = "spr.literalData";
constexpr const char* propSprOperation = "spr.operation";
} // namespace SprFrontend
