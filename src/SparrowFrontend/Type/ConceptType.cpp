#include <StdInc.h>
#include "ConceptType.h"
#include <Nodes/Decls/SprConcept.h>
#include <SparrowFrontend/SparrowFrontendTypes.h>

#include <Feather/Util/Decl.h>

#include <Nest/Common/Tuple.h>
#include <Nest/Intermediate/TypeStock.h>

using namespace SprFrontend;
using namespace Nest;

ConceptType* ConceptType::get(SprConcept* concept, uint8_t noReferences, Nest::EvalMode mode)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.autoType", "  Get ConceptType");

    TypeData* baseType = getConceptType(concept, noReferences, mode);

    return typeStock.getCreateType<ConceptType>(baseType);
}

SprConcept* ConceptType::concept() const
{
    return reinterpret_cast<SprConcept*>(data_->referredNode);
}
