#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Structure containing all the relevant information for a declaration instantiation
    /// The term instantiation refers here to an instance of a generic, a specialized declaration that was
    /// "instantiated" for a given set of CT arguments (values)
    class Instantiation : public DynNode
    {
        DEFINE_NODE(Instantiation, nkSparrowInnerInstantiation, "Sparrow.Inner.Instantiation");

    public:
        Instantiation(const Location& loc, DynNodeVector boundValues, DynNodeVector boundVars);

        /// The bound values - the values that define the instantiation as a particular case of a generic
        /// Usually this is a vector of CtValue nodes
        const DynNodeVector& boundValues() const;

        /// NodeList containing the variables & the actual instantiation
        /// This will contain variables for each bound value, and then the actual instantiated declaration
        Node*& expandedInstantiation();

        /// The actual declaration instantiation. This will be a part of 'expandedInstantiation'
        DynNode* instantiatedDecl();
        void setInstantiatedDecl(DynNode* decl);

        bool isValid() const;
        void setValid(bool valid = true);

    protected:
        void doSemanticCheck();
    };
}
