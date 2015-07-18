#include <StdInc.h>
#include "SparrowFrontendTypes.h"
#include <Feather/Util/Decl.h>

#include <SparrowFrontend/Nodes/Decls/SprConcept.h>

#include <Nest/Intermediate/TypeKindRegistrar.h>

namespace SprFrontend
{

namespace
{
    const char* getConceptTypeDescription(SprConcept* concept, uint8_t numReferences, EvalMode mode)
    {
        ostringstream os;
        if ( concept )
        {
            os << '#' << Feather::getName(concept->node());
        }
        else
        {
            os << "AnyType";
        }
        for ( uint8_t i=0; i<numReferences; ++i )
            os << '@';
        if ( mode == modeCt )
            os << "/ct";
        if ( mode == modeRtCt )
            os << "/rtct";
        return strdup(os.str().c_str());
    }

    TypeRef changeTypeModeConcept(TypeRef type, EvalMode newMode)
    {
        return getConceptType(((DynNode*) type->referredNode)->as<SprConcept>(), type->numReferences, newMode);
    }
}

int typeKindConcept = -1;

void initSparrowFrontendTypeKinds()
{
    typeKindConcept = registerTypeKind(&changeTypeModeConcept);
}

TypeRef getConceptType(SprConcept* concept, uint8_t numReferences, EvalMode mode)
{
    Type referenceType;
    referenceType.typeKind      = typeKindConcept;
    referenceType.mode          = mode;
    referenceType.numSubtypes   = 0;
    referenceType.numReferences = numReferences;
    referenceType.hasStorage    = 0;
    referenceType.canBeUsedAtCt = 1;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags         = 0;
    referenceType.referredNode  = concept->node();
    referenceType.description   = getConceptTypeDescription(concept, numReferences, mode);

    TypeRef t = findStockType(referenceType);
    if ( !t )
        t = insertStockType(referenceType);
    return t;
}


SprConcept* conceptOfType(TypeRef type)
{
    ASSERT(type && type->typeKind == typeKindConcept);
    return (SprConcept*) type->referredNode;
}

}
