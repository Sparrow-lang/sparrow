#pragma once

#include <vector>

FWD_CLASS1(SprFrontend, Instantiation);

namespace SprFrontend
{
    /// Implements a cache of instantiations. This is responsible for storing a set of generic instances, for checking
    /// the bound values to correspond with the existing instantiations, and to apply the if clause for the given
    /// instantiations
    class InstantiationsSet : public DynNode
    {
        DEFINE_NODE(InstantiationsSet, nkSparrowInnerInstantiationsSet, "Sparrow.Inner.InstantiationsSet");

    public:
        InstantiationsSet(DynNode* parentNode, DynNodeVector params, DynNode* ifClause);

        /// Checks if we can create an instantiation for the given values.
        /// It first consults the cache to determine if we already tried this combination of values. If this combination
        /// was not tried before, this will create an instantiation and checks the if clause. If the test is positive
        /// this will return the created instantiation. If the if clause validation fails, this returns nullptr.
        Instantiation* canInstantiate(const DynNodeVector& values, Nest::EvalMode evalMode);

        /// Getter for the CT parameters required for this instantiations set
        const DynNodeVector& parameters() const;

    private:
        /// Searches for an existing instantiation; returns null if not found
        Instantiation* searchInstantiation(const DynNodeVector& boundValues);

        /// Create a new instantiation - add the bounded variables
        Instantiation* createNewInstantiation(const DynNodeVector& boundValues, Nest::EvalMode evalMode);

        /// The node that uses this instantiations set
        DynNode* parentNode() const;

        /// The if clause corresponding to the generic
        DynNode* ifClause() const;

        /// The cache of instantiations
        vector<Instantiation*>& instantiations();
    };
}
