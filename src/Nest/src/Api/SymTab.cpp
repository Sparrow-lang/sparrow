#include "Nest/src/StdInc.hpp"
#include "Nest/Api/SymTab.h"

#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/NodeUtils.h"

using MMap = unordered_multimap<string, Nest_Node*>;
using MMapPair = MMap::value_type;

struct _SymTabImpl {
    Nest_SymTab base;
    MMap entries;
    MMap copiedEntries;
    vector<Nest_Node*> toCheckNodes;
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
    vector<Nest_Node*> nodesToCheck;
    nodesToCheck.swap(st->toCheckNodes);

    // One by one, compute their type
    // Here we may add additional symbols to this symbol table
    for (Nest_Node* n : nodesToCheck) {
        Nest_computeType(n);
    }
}
} // namespace

Nest_SymTab* Nest_mkSymTab(Nest_SymTab* parent, Nest_Node* node) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* res = (_SymTabImpl*)alloc(sizeof(_SymTabImpl), allocGeneral);
    res->base.parent = parent;
    res->base.node = node;
    new (&res->entries) MMap();
    new (&res->copiedEntries) MMap();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    return (Nest_SymTab*)res;
}

void Nest_symTabEnter(Nest_SymTab* symTab, const char* name, Nest_Node* node) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* st = (_SymTabImpl*)symTab;

    // Insert our entry in the normal map
    st->entries.insert(MMapPair(name, node));
    // Remove all the copied entries with the same name
    st->copiedEntries.erase(name);
}

void Nest_symTabAddToCheckNode(Nest_SymTab* symTab, Nest_Node* node) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* st = (_SymTabImpl*)symTab;
    st->toCheckNodes.push_back(node);
}

void Nest_symTabCopyEntries(Nest_SymTab* symTab, Nest_SymTab* otherSymTab) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* st = (_SymTabImpl*)symTab;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* other = (_SymTabImpl*)otherSymTab;

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

Nest_NodeArray Nest_symTabAllEntries(Nest_SymTab* symTab) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* st = (_SymTabImpl*)symTab;

    // Make sure to check all the required nodes for this symtab
    checkNodes(st);

    auto result = Nest_allocNodeArray(st->entries.size() + st->copiedEntries.size());
    for (auto entry : st->entries)
        Nest_appendNodeToArray(&result, entry.second);
    for (auto entry : st->copiedEntries)
        Nest_appendNodeToArray(&result, entry.second);
    return result;
}

Nest_NodeArray Nest_symTabLookupCurrent(Nest_SymTab* symTab, const char* name) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* st = (_SymTabImpl*)symTab;

    // Make sure to check all the required nodes for this symtab
    checkNodes(st);

    auto range = st->entries.equal_range(name);
    auto result = Nest_allocNodeArray(distance(range.first, range.second));
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

Nest_NodeArray Nest_symTabLookup(Nest_SymTab* symTab, const char* name) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* st = (_SymTabImpl*)symTab;

    // Make sure to check all the required nodes for this symtab
    checkNodes(st);

    auto res = Nest_symTabLookupCurrent(symTab, name);
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

void Nest_dumpSymTabs(Nest_SymTab* symTab) {
    int level = 0;
    for (; symTab; symTab = symTab->parent) {
        const Nest_StringRef* nameProp = nullptr;
        if (symTab->node)
            nameProp = Nest_getPropertyString(symTab->node, "name");
        const char* defName = nameProp ? nameProp->begin : "?";
        printf("===== Symbol table level %d - %s\n", level++, defName);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        auto* st = (_SymTabImpl*)symTab;
        dumpSortedEntries(st->entries);
        if (!st->copiedEntries.empty()) {
            printf("    + copied entries:\n");
            dumpSortedEntries(st->copiedEntries);
        }
    }
}

void Nest_dumpSymTabNames(Nest_SymTab* symTab) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* st = (_SymTabImpl*)symTab;
    dumpSortedEntries(st->entries, true);
    if (!st->copiedEntries.empty()) {
        printf(" + ");
        dumpSortedEntries(st->copiedEntries, true);
    }
}

void Nest_dumpSymTabHierarchy(Nest_SymTab* symTab) {
    printf("Symbol table hierarchy:");
    for (; symTab; symTab = symTab->parent) {
        const Nest_StringRef* nameProp = nullptr;
        if (symTab->node)
            nameProp = Nest_getPropertyString(symTab->node, "name");
        const char* defName = nameProp ? nameProp->begin : "?";
        printf(" %s", defName);
    }
    printf("\n");
}
