#include <StdInc.h>
#include "ConceptType.h"
#include <Nodes/Decls/SprConcept.h>

#include <Feather/Util/Decl.h>

#include <Nest/Common/Tuple.h>
#include <Nest/Intermediate/TypeStock.h>

using namespace SprFrontend;
using namespace Nest;

ConceptType::ConceptType(SprConcept* concept, uint8_t noReferences, Nest::EvalMode mode)
    : Type(typeConcept, mode)
{
    additionalData_ = concept;
    flags_ = noReferences;
    SET_TYPE_DESCRIPTION(*this);
}

ConceptType* ConceptType::get(SprConcept* concept, uint8_t noReferences, Nest::EvalMode mode)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.autoType", "  Get ConceptType");

    typedef Tuple3<SprConcept*, uint8_t, int> Key;
    Key key(concept, noReferences, mode);

    return Nest::typeStock.getCreateType<ConceptType*>(typeConcept, key,
        [](void* p, const Key& key) { return new (p) ConceptType(key._1, key._2, (Nest::EvalMode) key._3); } );
}

SprConcept* ConceptType::concept() const
{
    return reinterpret_cast<SprConcept*>(additionalData_);
}

uint8_t ConceptType::noReferences() const
{
    return (uint8_t) flags_;
}

string ConceptType::toString() const
{
    ostringstream os;
    SprConcept* c = concept();
    if ( c )
    {
        os << '#' << Feather::getName(c);
    }
    else
    {
        os << "auto";
    }
    for ( uint8_t i=0; i<flags_; ++i )
        os << '@';
    if ( mode() == modeCt )
        os << "/ct";
    if ( mode() == modeRtCt )
        os << "/rtct";
    return os.str();
}

ConceptType* ConceptType::changeMode(EvalMode newMode)
{
    return ConceptType::get(concept(), noReferences(), newMode);
}
