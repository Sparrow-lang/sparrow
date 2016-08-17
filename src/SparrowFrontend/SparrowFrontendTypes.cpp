#include <StdInc.h>
#include "SparrowFrontendTypes.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Api/TypeKindRegistrar.h"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/Alloc.h"

namespace SprFrontend
{

namespace
{
    const char* getConceptTypeDescription(Node* concept, uint8_t numReferences, EvalMode mode)
    {
        ostringstream os;
        for ( uint8_t i=0; i<numReferences; ++i )
            os << '@';
        if ( concept )
        {
            os << '#' << toString(Feather_getName(concept));
        }
        else
        {
            os << "AnyType";
        }
        if ( mode == modeCt )
            os << "/ct";
        if ( mode == modeRtCt )
            os << "/rtct";
        return dupString(os.str().c_str());
    }

    TypeRef changeTypeModeConcept(TypeRef type, EvalMode newMode)
    {
        return getConceptType(Nest_ofKind(type->referredNode, nkSparrowDeclSprConcept), type->numReferences, newMode);
    }
}

int typeKindConcept = -1;

void initSparrowFrontendTypeKinds()
{
    typeKindConcept = Nest_registerTypeKind(&changeTypeModeConcept);
}

TypeRef getConceptType(Node* concept, uint8_t numReferences, EvalMode mode)
{
    ASSERT(!concept || concept->nodeKind == nkSparrowDeclSprConcept);
    Type referenceType;
    referenceType.typeKind      = typeKindConcept;
    referenceType.mode          = mode;
    referenceType.numSubtypes   = 0;
    referenceType.numReferences = numReferences;
    referenceType.hasStorage    = 0;
    referenceType.canBeUsedAtCt = 1;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags         = 0;
    referenceType.referredNode  = concept;
    referenceType.description   = getConceptTypeDescription(concept, numReferences, mode);

    TypeRef t = Nest_findStockType(&referenceType);
    if ( !t )
        t = Nest_insertStockType(&referenceType);
    return t;
}


Node* conceptOfType(TypeRef type)
{
    ASSERT(type && type->typeKind == typeKindConcept);
    return type->referredNode;
}

}
