#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>


FWD_CLASS1(SprFrontend, InstantiationsSet);

namespace SprFrontend
{
    /// A Sparrow concept declaration
    class SprConcept : public Node
    {
        DEFINE_NODE(SprConcept, nkSparrowDeclSprConcept, "Sparrow.Decl.Concept");

    public:
        SprConcept(const Location& loc, string name, string paramName, Node* baseConcept, Node* ifClause, AccessType accessType = publicAccess);

        /// Checks if the concept is fulfilled for the given type
        bool isFulfilled(Type* type);

        /// Returns the base concept type; the type of the variable for this concept
        Type* baseConceptType() const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doSemanticCheck();
    };
}
