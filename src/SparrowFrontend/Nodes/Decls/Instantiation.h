#pragma once

#include <vector>

namespace SprFrontend
{
    /// Structure containing all the relevant information for a declaration instantiation
    /// The term instantiation refers here to an instance of a generic, a specialized declaration that was
    /// "instantiated" for a given set of CT arguments (values)
    class Instantiation
    {
    public:
        /// The bound values - the values that define the instantiation as a particular case of a generic
        /// Usually this is a vector of CtValue nodes
        NodeVector boundValues_;

        /// NodeList containing the variables & the actual instantiation
        /// This will contain variables for each bound value, and then the actual instantiated declaration
        NodeList* expandedInstantiation_;
    
        /// The actual declaration instantiation. This will be a part of 'expandedInstantiation_'
        Node* instantiatedDecl_;

        /// This flag will be set when all the checks are passed and this is a valid instantiation
        bool isValid_;

        Instantiation()
            : expandedInstantiation_(nullptr)
            , instantiatedDecl_(nullptr)
            , isValid_(false)
        {}
    };
}
