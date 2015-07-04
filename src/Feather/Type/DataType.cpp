#include <StdInc.h>
#include "DataType.h"
#include <Nodes/Properties.h>
#include <Nodes/Decls/Class.h>
#include <FeatherTypes.h>
#include <Util/Decl.h>

#include <Nest/Common/Diagnostic.h>
#include <Nest/Common/Tuple.h>
#include <Nest/Intermediate/TypeStock.h>

using namespace Feather;
using namespace Nest;

DataType* DataType::get(Class* classDecl, uint8_t noReferences, EvalMode mode)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.dataType", "  Get DataType");

    TypeData* baseType = getDataType(classDecl, noReferences, mode);

    return typeStock.getCreateType<DataType>(baseType);
}
