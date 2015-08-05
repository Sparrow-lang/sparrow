#include <StdInc.h>
#include "Ct.h"
#include "Overload.h"
#include "SprTypeTraits.h"
#include "StdDef.h"
#include <NodeCommonsCpp.h>

#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Ct.h>

using namespace Feather;
using namespace Nest;

namespace
{
    template <typename ValueType>
    ValueType evalValue(Node* node, TypeRef expectedExpType)
    {
        node = theCompiler().ctEval(node);
        TypeRef t = removeLValueIfPresent(node->type);
        if ( !isSameTypeIgnoreMode(t, expectedExpType) )
            REP_INTERNAL(node->location, "Invalid value; found expression of type %1%, expected %2%") % node->type % expectedExpType;
        CHECK(node->location, node->nodeKind == Feather::nkFeatherExpCtValue);
        ValueType* val = node ? getCtValueData<ValueType>(node) : nullptr;
        if ( !val )
            REP_ERROR(node->location, "Invalid value");
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
    NodeVector decls = context->currentSymTab()->lookup("==");
    if ( !decls.empty() )
    {
        Node* funCall = selectOverload(context, v1->location, modeCt, move(decls), {v1, v2}, false, "");
        if ( funCall )
        {
            Nest::semanticCheck(funCall);
            if ( Feather::isTestable(funCall) && Feather::isCt(funCall) )
            {
                Node* c = theCompiler().ctEval(funCall);
                return SprFrontend::getBoolCtValue(c);
            }
        }
    }

    // Just compare the values
    return sameCtValue(v1, v2);
}

const char* SprFrontend::getStringCtValue(Node* val)
{
    return evalValue<const char*>(val, StdDef::typeStringRef);
}
bool SprFrontend::getBoolCtValue(Node* val)
{
    return evalValue<bool>(val, StdDef::typeBool);
}
int SprFrontend::getIntCtValue(Node* val)
{
    return evalValue<int>(val, StdDef::typeInt);
}
int* SprFrontend::getIntRefCtValue(Node* val)
{
    return evalValue<int*>(val, StdDef::typeRefInt);
}
size_t SprFrontend::getSizeTypeCtValue(Node* val)
{
    return evalValue<size_t>(val, StdDef::typeSizeType);
}
