#include "Feather/src/StdInc.h"
#include "Feather/src/Api/Feather_Types.h"
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

using namespace Feather;

int typeKindVoid = -1;
int typeKindData = -1;
int typeKindPtr = -1;
int typeKindConst = -1;
int typeKindMutable = -1;
int typeKindTemp = -1;
int typeKindArray = -1;
int typeKindFunction = -1;

void initFeatherTypeKinds() {
    typeKindVoid = VoidType::registerTypeKind();
    typeKindData = DataType::registerTypeKind();
    typeKindPtr = PtrType::registerTypeKind();
    typeKindConst = ConstType::registerTypeKind();
    typeKindMutable = MutableType::registerTypeKind();
    typeKindTemp = TempType::registerTypeKind();
    typeKindArray = ArrayType::registerTypeKind();
    typeKindFunction = FunctionType::registerTypeKind();
}
