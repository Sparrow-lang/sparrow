#pragma once

#include "SymTab.h"

namespace Nest
{
    class SymTabEntryImpl;

    /// Class that represents a symbol table
    class SymTabImpl : public SymTab
    {
    public:
        SymTabImpl(SymTab* parent, Node* node);
        virtual ~SymTabImpl();

        virtual void enter(const string& name, Node* definition);
        virtual void copyEntries(SymTab* otherSymTab);
        virtual NodeVector allEntries() const;
        virtual NodeVector lookupCurrent(const string& name) const;
        virtual NodeVector lookup(const string& name) const;

        virtual SymTab* parent() const;
        virtual Node* node() const;

        virtual void dump(ostream& os) const;

    private:
        void dumpImpl(ostream& os, int nestLevel = 0) const;

    private:
        /// The parent symbol table
        SymTab* parent_;

        /// The node that introduced this symbol table
        Node* node_;

        /// The entries in this symbol table (not copied)
        unordered_multimap<string, Node*> entries_;

        /// The entries in this symbol table that are copied. They must be shadowed by 'entries_'
        unordered_multimap<string, Node*> copiedEntries_;
    };
}
