#include <StdInc.h>
#include "SparrowFrontend/Services/Callable/ConceptCallable.h"
#include "SparrowFrontend/Services/IConceptsService.h"
#include "SparrowFrontend/Helpers/StdDef.h"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace SprFrontend {

ConceptCallable::ConceptCallable(ConceptDecl concept)
    : Callable(concept)
    , paramType_(ConceptType::get()) {}

ConversionType ConceptCallable::canCall(const CCLoc& ccloc, NodeRange args, EvalMode evalMode,
        CustomCvtMode customCvtMode, bool reportErrors) {
    NodeVector args2;

    // Expecting exactly one argument
    if (args.size() != 1) {

        if (reportErrors)
            REP_INFO(NOLOC, "Expecting one argument; given=%1%") % args.size();
        return convNone;
    }

    // Save the argument for 'generateCall'
    argument_ = args[0];
    if (!argument_.computeType())
        return convNone;

    return convConcept; // Always succeeds, regardless of the type of the argument
}
ConversionType ConceptCallable::canCall(const CCLoc& ccloc, Range<Type> argTypes,
        EvalMode evalMode, CustomCvtMode customCvtMode, bool reportErrors) {
    // Always succeed
    return convConcept;
}

NodeHandle ConceptCallable::generateCall(const CCLoc& ccloc) {
    // Get the base type, and remove all the references
    Type argType = argument_.type();
    ASSERT(argType);
    if (argType.hasStorage())
        argType = Feather::removeAllRefs(TypeWithStorage(argType));

    // Check if the type of the argument fulfills the concept
    bool conceptFulfilled = g_ConceptsService->conceptIsFulfilled(ConceptDecl(decl_), argType);

    // Return a compile-time boolean value
    auto boolValue = Feather::CtValueExp::createT(ccloc.loc_, StdDef::typeBool, conceptFulfilled);
    boolValue.setContext(ccloc.context_);
    return boolValue.semanticCheck();
}

int ConceptCallable::numParams() const { return 1; }

Type ConceptCallable::paramType(int idx) const {
    ASSERT(idx == 0);
    return paramType_;
}

string ConceptCallable::toString() const {
    ostringstream oss;
    oss << decl_.name();
    return oss.str();
}

} // namespace SprFrontend
