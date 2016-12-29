#include <StdInc.h>
#include "Ct.h"
#include "Overload.h"
#include "SprTypeTraits.h"
#include "StdDef.h"
#include <NodeCommonsCpp.h>

#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Utils/NodeUtils.hpp"

using namespace Nest;

namespace
{
    template <typename ValueType>
    ValueType evalValue(Node* node, TypeRef expectedExpType)
    {
        node = Nest_ctEval(node);
        TypeRef t = Feather_removeLValueIfPresent(node->type);
        if ( !Feather_isSameTypeIgnoreMode(t, expectedExpType) )
            REP_INTERNAL(node->location, "Invalid value; found expression of type %1%, expected %2%") % node->type % expectedExpType;
        CHECK(node->location, node->nodeKind == nkFeatherExpCtValue);
        ValueType* val = node ? Feather_getCtValueData<ValueType>(node) : nullptr;
        if ( !val )
            REP_INTERNAL(node->location, "Invalid value");
        return *val;
    }
}

bool SprFrontend::ctValsEqual(Node* v1, Node* v2)
{
    if ( v1->nodeKind != nkFeatherExpCtValue )
        REP_INTERNAL(v1->location, "CtValue required when comparing CT value equality");
    if ( v2->nodeKind != nkFeatherExpCtValue )
        REP_INTERNAL(v1->location, "CtValue required when comparing CT value equality");

    CompilationContext* context = v1->context;

    ASSERT(v1->type);
    ASSERT(v2->type);

    if ( v1->type != v2->type )
        return false;

    // Check if we are comparing type values
    TypeRef t1 = tryGetTypeValue(v1);
    if ( t1 )
    {
        TypeRef t2 = tryGetTypeValue(v2);
        if ( t2 )
            return t1 == t2;
    }

    // Check if we can call the '==' operator
    // If we can call it, then actually call it and return the result
    NodeArray decls = Nest_symTabLookup(context->currentSymTab, "==");
    if ( Nest_nodeArraySize(decls) > 0 )
    {
        Node* funCall = selectOverload(context, v1->location, modeCt, all(decls), fromIniList({v1, v2}), OverloadReporting::none, fromCStr(""));
        Nest_freeNodeArray(decls);
        if ( funCall )
        {
            Nest_semanticCheck(funCall);
            if ( Feather_isTestable(funCall) && Feather_isCt(funCall) )
            {
                Node* c = Nest_ctEval(funCall);
                return SprFrontend::getBoolCtValue(c);
            }
        }
    }

    // Just compare the values
    if ( v1->nodeKind != nkFeatherExpCtValue )
        REP_INTERNAL(v1->location, "Invalid CtValue");
    if ( v1->nodeKind != nkFeatherExpCtValue )
        REP_INTERNAL(v2->location, "Invalid CtValue");
    return Nest_getCheckPropertyType(v1, "valueType") == Nest_getCheckPropertyType(v2, "valueType")
        && Nest_getCheckPropertyString(v1, "valueData") == Nest_getCheckPropertyString(v2, "valueData");
}

StringRef SprFrontend::getStringCtValue(Node* val)
{
    return evalValue<StringRef>(val, StdDef::typeStringRef);
}
bool SprFrontend::getBoolCtValue(Node* val)
{
    return evalValue<bool>(val, StdDef::typeBool);
}
int SprFrontend::getIntCtValue(Node* val)
{
    return evalValue<int>(val, StdDef::typeInt);
}
char* SprFrontend::getByteRefCtValue(Node* val)
{
    return evalValue<char*>(val, StdDef::typeRefByte);
}
size_t SprFrontend::getSizeTypeCtValue(Node* val)
{
    return evalValue<size_t>(val, StdDef::typeSizeType);
}
