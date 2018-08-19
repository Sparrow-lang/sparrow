#pragma once

#include "Nest/Api/StringRef.h"

typedef struct Nest_Modifier Nest_Modifier;

Nest_Modifier* SprFe_getPublicMod();
Nest_Modifier* SprFe_getProtectedMod();
Nest_Modifier* SprFe_getPrivateMod();
Nest_Modifier* SprFe_getCtMod();
Nest_Modifier* SprFe_getRtMod();
Nest_Modifier* SprFe_getAutoCtMod();
Nest_Modifier* SprFe_getCtGenericMod();
Nest_Modifier* SprFe_getNativeMod(StringRef name);
Nest_Modifier* SprFe_getConvertMod();
Nest_Modifier* SprFe_getNoDefaultMod();
Nest_Modifier* SprFe_getInitCtorMod();
Nest_Modifier* SprFe_getMacroMod();
Nest_Modifier* SprFe_getNoInlineMod();

bool SprFe_isEvalModeMod(Nest_Modifier* mod);