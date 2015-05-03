#pragma once

#include <Nest/Intermediate/Type.h>

#include <vector>

FWD_CLASS1(SprFrontend, SprConcept);

namespace SprFrontend
{
    /// A type that represents a generic set of types, types that model a given concept. If no concept is given, the
    /// type will model any type
    class ConceptType : public Nest::Type
    {
    public:
        static ConceptType* get(SprConcept* concept = nullptr, uint8_t noReferences = 0, Nest::EvalMode mode = Nest::modeRtCt);

        /// The concept used for this type. Can be nullptr if no concept is used
        SprConcept* concept() const;
        
        /// The number of references applied
        virtual uint8_t noReferences() const;
        
        virtual string toString() const;
        virtual ConceptType* changeMode(EvalMode newMode);

    private:
        explicit ConceptType(SprConcept* concept, uint8_t noReferences, Nest::EvalMode mode);
    };
}
