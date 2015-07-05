#include <StdInc.h>
#include "SparrowFrontendTypes.h"
#include <Feather/Util/Decl.h>

#include <SparrowFrontend/Nodes/Decls/SprConcept.h>

namespace SprFrontend
{

namespace
{
    const char* getConceptTypeDescription(SprConcept* concept, uint8_t numReferences, EvalMode mode)
    {
        ostringstream os;
        if ( concept )
        {
            os << '#' << Feather::getName(concept);
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
}

TypeRef getConceptType(SprConcept* concept, uint8_t numReferences, EvalMode mode)
{
    Type referenceType;
    referenceType.typeId        = Nest::typeConcept;
    referenceType.mode          = mode;
    referenceType.numSubtypes   = 0;
    referenceType.numReferences = numReferences;
    referenceType.hasStorage    = 0;
    referenceType.canBeUsedAtCt = 1;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags         = 0;
    referenceType.referredNode  = concept;
    referenceType.description   = getConceptTypeDescription(concept, numReferences, mode);

    TypeRef t = findStockType(referenceType);
    if ( !t )
        t = insertStockType(referenceType);
    return t;
}


SprConcept* conceptOfType(TypeRef type)
{
    ASSERT(type && type->typeId == typeConcept);
    return type->referredNode->as<SprConcept>();
}

}
