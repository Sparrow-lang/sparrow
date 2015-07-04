#include <StdInc.h>
#include "DataType.h"
#include <Nodes/Properties.h>
#include <Nodes/Decls/Class.h>
#include <Util/Decl.h>

#include <Nest/Common/Diagnostic.h>
#include <Nest/Common/Tuple.h>
#include <Nest/Intermediate/TypeStock.h>

using namespace Feather;
using namespace Nest;

DataType::DataType(Class* classDecl, uint8_t noReferences, EvalMode mode)
    : StorageType(typeData, mode)
{
    data_.numReferences = noReferences;
    data_.hasStorage    = 1;
    data_.canBeUsedAtRt = effectiveEvalMode(classDecl) != modeCt;
    data_.canBeUsedAtCt = effectiveEvalMode(classDecl) != modeRt;
    data_.referredNode  = classDecl;
    SET_TYPE_DESCRIPTION(*this);
}

DataType* DataType::get(Class* classDecl, uint8_t noReferences, EvalMode mode)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.dataType", "  Get DataType");

    typedef Tuple3<Class*, uint8_t, int> DataTypeKey;

    if ( mode == modeRtCt && classDecl )
        mode = effectiveEvalMode(classDecl);

    DataTypeKey key(classDecl, noReferences, (int) mode);

    return typeStock.getCreateType<DataType*>(typeData, key,
        [](void* p, const DataTypeKey& key) { return new (p) DataType(key._1, key._2, (EvalMode) key._3); } );
}

string DataType::toString() const
{
    Class* cls = classDecl();
    uint8_t n = noReferences();
    string res;
    for ( uint8_t i=0; i<n; ++i )
        res += '@';
    if ( cls )
    {
        const string* description = cls->getPropertyString(propDescription);
        res += description ? *description : getName(cls);
    }
    else
        res += "<no class>";
    if ( mode() == modeCt )
        res += "/ct";
    if ( mode() == modeRtCt )
        res += "/rtct";
    return res;
}

DataType* DataType::changeMode(EvalMode newMode)
{
    return DataType::get(classDecl(), noReferences(), newMode);
}
