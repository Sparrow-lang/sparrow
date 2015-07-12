#pragma once

#include <Nest/Intermediate/NodeVector.h>

namespace Nest
{
    /// Class that represents a symbol table
    ///
    /// A symbol table contains a list of symbol table entries (with different names). It holds reference to the
    /// parent symbol table that contains this symbol table, therefore holding the stack of symbol tables until the
    /// current one. It also holds the node that introduced this symbol table
    ///
    /// This class provides helper methods for adding new entries to the symbol table, looking for entries in the symbol
    /// table, etc.
    class SymTab
    {
    public:
        SymTab() {};
        virtual ~SymTab() {};

        /// Create and enter a new entry into the symbol table
        virtual void enter(const string& name, Node* node) = 0;

        /// Copies the entries from the given tab to the current tab
        /// If this symbol table already contains the given symbols, this will not add them anymore. However, if this
        /// table contains some copied entries, this will add new entries with the same name
        virtual void copyEntries(SymTab* otherSymTab) = 0;

        /// Returns a list of symbol table entries
        virtual NodeVector allEntries() const = 0;
        virtual DynNodeVector allEntriesDyn() const = 0;

        /// Look up an existing symbol table entry, by the name of the symbol
        /// Note that more than one symbols can be registered in the table with the same name
        virtual NodeVector lookupCurrent(const string& name) const = 0;
        virtual DynNodeVector lookupCurrentDyn(const string& name) const = 0;

        /// Look up an existing symbol table entry and parent entries, by the name of the symbol
        /// If no matching symbol is found in the current symbol table, this will recursively try in the parent tables.
        /// Note that more than one symbols can be registered in the table with the same name
        virtual NodeVector lookup(const string& name) const = 0;
        virtual DynNodeVector lookupDyn(const string& name) const = 0;


        /// Returns the symbol table that contains this table. May return null.
        virtual SymTab* parent() const = 0;

        /// Returns the node that created this symbol table
        virtual Node* node() const = 0;

        /// Dumps the information of this symbol table to the given stream
        virtual void dump(ostream& os) const = 0;
    };

    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, const SymTab& n)
    {
        n.dump(os);
        return os;
    }

    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, const SymTab* n)
    {
        if ( n )
            n->dump(os);
        return os;
    }
}
