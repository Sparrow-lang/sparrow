#pragma once

#include "SymTab.h"

namespace Nest
{
    class SymTabEntryImpl;

    /// Class that represents a symbol table
    class SymTabImpl : public SymTab
    {
    public:
        SymTabImpl(SymTab* parent, DynNode* node);
        virtual ~SymTabImpl();

        virtual void enter(const string& name, DynNode* definition);
        virtual void copyEntries(SymTab* otherSymTab);
        virtual DynNodeVector allEntries() const;
        virtual DynNodeVector lookupCurrent(const string& name) const;
        virtual DynNodeVector lookup(const string& name) const;

        virtual SymTab* parent() const;
        virtual DynNode* node() const;

        virtual void dump(ostream& os) const;

    private:
        void dumpImpl(ostream& os, int nestLevel = 0) const;

    private:
        /// The parent symbol table
        SymTab* parent_;

        /// The node that introduced this symbol table
        DynNode* node_;

        /// The entries in this symbol table (not copied)
        unordered_multimap<string, DynNode*> entries_;

        /// The entries in this symbol table that are copied. They must be shadowed by 'entries_'
        unordered_multimap<string, DynNode*> copiedEntries_;
    };
}
