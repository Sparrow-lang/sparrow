#pragma once

#include <Nest/Intermediate/NodeVector.h>

/// Structure describing a symbol table, with multiple entries
/// Note that the entries will be allocated after this structure.
struct Nest_SymTab {
    struct Nest_SymTab* parent;                     ///< The parent symbol table
    Nest::Node* node;                               ///< The node that introduced this symbol table
};

typedef struct Nest_SymTab Nest_SymTab;
typedef struct Nest_SymTab SymTab;


/// Creates a SymTab object, for the given parent and given node
SymTab* Nest_mkSymTab(SymTab* parent, Nest::Node* node);

/// Create and enter a new entry into the symbol table
void Nest_symTabEnter(SymTab* symTab, const char* name, Nest::Node* node);

/// Copies the entries from the given tab to the current tab
/// If this symbol table already contains the given symbols, this will not add them anymore. However, if this
/// table contains some copied entries, this will add new entries with the same name
void Nest_symTabCopyEntries(SymTab* symTab, SymTab* otherSymTab);

/// Returns a list of symbol table entries
Nest::NodeVector Nest_symTabAllEntries(SymTab* symTab);

/// Look up an existing symbol table entry, by the name of the symbol
/// Note that more than one symbols can be registered in the table with the same name
Nest::NodeVector Nest_symTabLookupCurrent(SymTab* symTab, const char* name);

/// Look up an existing symbol table entry and parent entries, by the name of the symbol
/// If no matching symbol is found in the current symbol table, this will recursively try in the parent tables.
/// Note that more than one symbols can be registered in the table with the same name
Nest::NodeVector Nest_symTabLookup(SymTab* symTab, const char* name);
