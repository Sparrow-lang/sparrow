#pragma once

#include "Nest/Api/StringRef.h"

typedef struct Nest_Modifier Modifier;

Modifier* SprFe_getStaticMod();
Modifier* SprFe_getCtMod();
Modifier* SprFe_getRtMod();
Modifier* SprFe_getRtCtMod();
Modifier* SprFe_getAutoCtMod();
Modifier* SprFe_getCtGenericMod();
Modifier* SprFe_getNativeMod(StringRef name);
Modifier* SprFe_getConvertMod();
Modifier* SprFe_getNoDefaultMod();
Modifier* SprFe_getInitCtorMod();
Modifier* SprFe_getMacroMod();
Modifier* SprFe_getNoInlineMod();

bool SprFe_isEvalModeMod(Modifier* mod);