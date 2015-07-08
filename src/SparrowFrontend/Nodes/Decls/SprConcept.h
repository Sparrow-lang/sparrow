#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>


FWD_CLASS1(SprFrontend, InstantiationsSet);

namespace SprFrontend
{
    /// A Sparrow concept declaration
    class SprConcept : public DynNode
    {
        DEFINE_NODE(SprConcept, nkSparrowDeclSprConcept, "Sparrow.Decl.Concept");

    public:
        SprConcept(const Location& loc, string name, string paramName, DynNode* baseConcept, DynNode* ifClause, AccessType accessType = publicAccess);

        /// Checks if the concept is fulfilled for the given type
        bool isFulfilled(TypeRef type);

        /// Returns the base concept type; the type of the variable for this concept
        TypeRef baseConceptType() const;

    protected:
        void doSetContextForChildren();
        void doSemanticCheck();
    };
}
