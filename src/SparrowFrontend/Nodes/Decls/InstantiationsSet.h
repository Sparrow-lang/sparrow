#pragma once

#include <vector>

FWD_CLASS1(SprFrontend, Instantiation);

namespace SprFrontend
{
    /// Implements a cache of instantiations. This is responsible for storing a set of generic instances, for checking
    /// the bound values to correspond with the existing instantiations, and to apply the if clause for the given
    /// instantiations
    class InstantiationsSet
    {
    public:
        InstantiationsSet(Node* parentNode, NodeVector params, Node* ifClause);
        ~InstantiationsSet();

        /// Checks if we can create an instantiation for the given values.
        /// It first consults the cache to determine if we already tried this combination of values. If this combination
        /// was not tried before, this will create an instantiation and checks the if clause. If the test is positive
        /// this will return the created instantiation. If the if clause validation fails, this returns nullptr.
        Instantiation* canInstantiate(const NodeVector& values, Nest::EvalMode evalMode);

        /// Getter for the CT parameters required for this instantiations set
        const NodeVector& parameters() const { return params_; }

    private:
        /// Searches for an existing instantiation; returns null if not found
        Instantiation* searchInstantiation(const NodeVector& boundValues);

        /// Create a new instantiation - add the bounded variables
        Instantiation* createNewInstantiation(const NodeVector& boundValues, Nest::EvalMode evalMode);

    private:
        /// The node that uses this instantiations set
        Node* parentNode_;

        /// The parameters corresponding to the checked CT values
        NodeVector params_;

        /// The if clause corresponding to the generic
        Node* ifClause_;

        /// The cache of instantiations
        vector<Instantiation*> instantiations_;
    };
}
