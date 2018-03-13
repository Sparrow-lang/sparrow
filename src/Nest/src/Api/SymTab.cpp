#include "Nest/src/StdInc.hpp"
#include "Nest/Api/SymTab.h"

#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/NodeUtils.h"

#include <boost/bind.hpp>

typedef unordered_multimap<string, Node*> MMap;
typedef MMap::value_type MMapPair;

struct _SymTabImpl {
    SymTab base;
    MMap entries;
    MMap copiedEntries;
    vector<Node*> toCheckNodes;
};

template <class ITERATOR> ITERATOR begin(std::pair<ITERATOR, ITERATOR>& range) {
    return range.first;
}

template <class ITERATOR> ITERATOR end(std::pair<ITERATOR, ITERATOR>& range) {
    return range.second;
}

namespace {
void checkNodes(_SymTabImpl* st) {
    if (st->toCheckNodes.empty())
        return;

    // Get all the nodes that we need to check
    vector<Node*> nodesToCheck;
    nodesToCheck.swap(st->toCheckNodes);

    // One by one, compute their type
    // Here we may add additional symbols to this symbol table
    for (Node* n : nodesToCheck) {
        Nest_computeType(n);
    }
}
} // namespace

SymTab* Nest_mkSymTab(SymTab* parent, Node* node) {
    _SymTabImpl* res = (_SymTabImpl*)alloc(sizeof(_SymTabImpl), allocGeneral);
    res->base.parent = parent;
    res->base.node = node;
    new (&res->entries) MMap();
    new (&res->copiedEntries) MMap();
    return (SymTab*)res;
}

void Nest_symTabEnter(SymTab* symTab, const char* name, Node* node) {
    _SymTabImpl* st = (_SymTabImpl*)symTab;

    // Insert our entry in the normal map
    st->entries.insert(MMapPair(name, node));
    // Remove all the copied entries with the same name
    st->copiedEntries.erase(name);
}

void Nest_symTabAddToCheckNode(SymTab* symTab, Node* node) {
    _SymTabImpl* st = (_SymTabImpl*)symTab;
    st->toCheckNodes.push_back(node);
}

void Nest_symTabCopyEntries(SymTab* symTab, SymTab* otherSymTab) {
    _SymTabImpl* st = (_SymTabImpl*)symTab;
    _SymTabImpl* other = (_SymTabImpl*)otherSymTab;

    for (const auto& entry : other->entries) {
        // Insert the item into our set of copied entries, only if we don't find it in our normal
        // set This way, all the copied elements are shadowed by the normal entries
        auto it = st->entries.find(entry.first);
        if (it == st->entries.end()) {
            st->copiedEntries.insert(entry);
        }
    }
    for (const auto& entry : other->copiedEntries) {
        // Insert the item into our set of copied entries, only if we don't find it in our normal
        // set This way, all the copied elements are shadowed by the normal entries
        auto it = st->entries.find(entry.first);
        if (it == st->entries.end()) {
            st->copiedEntries.insert(entry);
        }
    }
}

NodeArray Nest_symTabAllEntries(SymTab* symTab) {
    _SymTabImpl* st = (_SymTabImpl*)symTab;

    // Make sure to check all the required nodes for this symtab
    checkNodes(st);

    NodeArray result = Nest_allocNodeArray(st->entries.size() + st->copiedEntries.size());
    for (auto entry : st->entries)
        Nest_appendNodeToArray(&result, entry.second);
    for (auto entry : st->copiedEntries)
        Nest_appendNodeToArray(&result, entry.second);
    return result;
}

NodeArray Nest_symTabLookupCurrent(SymTab* symTab, const char* name) {
    _SymTabImpl* st = (_SymTabImpl*)symTab;

    // Make sure to check all the required nodes for this symtab
    checkNodes(st);

    auto range = st->entries.equal_range(name);
    NodeArray result = Nest_allocNodeArray(distance(range.first, range.second));
    for (auto entry : range)
        Nest_appendNodeToArray(&result, entry.second);
    if (Nest_nodeArraySize(result) == 0) {
        auto range = st->copiedEntries.equal_range(name);
        for (auto entry : range)
            Nest_appendNodeToArray(&result, entry.second);
    }

    // Remove duplicate entries
    sort(result.beginPtr, result.endPtr);
    result.endPtr = unique(result.beginPtr, result.endPtr);

    return result;
}

NodeArray Nest_symTabLookup(SymTab* symTab, const char* name) {
    _SymTabImpl* st = (_SymTabImpl*)symTab;

    // Make sure to check all the required nodes for this symtab
    checkNodes(st);

    NodeArray res = Nest_symTabLookupCurrent(symTab, name);
    return Nest_nodeArraySize(res) > 0 || !st->base.parent
                   ? res
                   : Nest_symTabLookup(st->base.parent, name);
}

namespace {
void dumpEntry(const char* name, int numEntries, bool isFirst, bool singleLine) {
    if (singleLine && !isFirst)
        printf(", ");
    if (!singleLine)
        printf("    ");
    if (numEntries > 1)
        printf("%s (%d times)", name, numEntries);
    else
        printf("%s", name);
    if (!singleLine)
        printf("\n");
}
void dumpSortedEntries(MMap& entriesMap, bool singleLine = false) {
    if (entriesMap.empty())
        return;
    // Gather all the entries
    vector<string> entries;
    entries.reserve(entriesMap.size());
    for (auto entry : entriesMap)
        entries.push_back(entry.first);
    // Sort them
    sort(entries.begin(), entries.end());
    // Print them in groups, avoiding duplicates
    int numEntries = 0;
    int startIdx = 0;
    for (int i = 0; i < int(entries.size()); ++i) {
        if (entries[startIdx] != entries[i]) {
            // Print the last entry
            dumpEntry(entries[startIdx].c_str(), numEntries, startIdx == 0, singleLine);
            // update the state vars
            numEntries = 1;
            startIdx = i;
        } else {
            ++numEntries;
        }
    }
    // Print the last entry
    dumpEntry(entries[startIdx].c_str(), numEntries, startIdx == 0, singleLine);
}
} // namespace

void Nest_dumpSymTabs(SymTab* symTab) {
    int level = 0;
    for (; symTab; symTab = symTab->parent) {
        const StringRef* nameProp = nullptr;
        if (symTab->node)
            nameProp = Nest_getPropertyString(symTab->node, "name");
        const char* defName = nameProp ? nameProp->begin : "?";
        printf("===== Symbol table level %d - %s\n", level++, defName);

        _SymTabImpl* st = (_SymTabImpl*)symTab;
        dumpSortedEntries(st->entries);
        if (!st->copiedEntries.empty()) {
            printf("    + copied entries:\n");
            dumpSortedEntries(st->copiedEntries);
        }
    }
}

void Nest_dumpSymTabNames(SymTab* symTab) {
    _SymTabImpl* st = (_SymTabImpl*)symTab;
    dumpSortedEntries(st->entries, true);
    if (!st->copiedEntries.empty()) {
        printf(" + ");
        dumpSortedEntries(st->copiedEntries, true);
    }
}

void Nest_dumpSymTabHierarchy(SymTab* symTab) {
    printf("Symbol table hierarchy:");
    for (; symTab; symTab = symTab->parent) {
        const StringRef* nameProp = nullptr;
        if (symTab->node)
            nameProp = Nest_getPropertyString(symTab->node, "name");
        const char* defName = nameProp ? nameProp->begin : "?";
        printf(" %s", defName);
    }
    printf("\n");
}
