#include <StdInc.h>
#include "SymTab.h"

#include "Common/Alloc.h"

#include <boost/bind.hpp>

using namespace Nest;

typedef unordered_multimap<string, Node*> MMap;
typedef MMap::value_type MMapPair;

struct _SymTabImpl {
    SymTab base;
    MMap entries;
    MMap copiedEntries;
};


SymTab* Nest_mkSymTab(SymTab* parent, Node* node)
{
    _SymTabImpl* res = (_SymTabImpl*) alloc(sizeof(_SymTabImpl), allocGeneral);
    res->base.parent = parent;
    res->base.node = node;
    new (&res->entries) MMap();
    new (&res->copiedEntries) MMap();
    return (SymTab*) res;
}

void Nest_symTabEnter(SymTab* symTab, const char* name, Node* node)
{
    _SymTabImpl* st = (_SymTabImpl*) symTab;

    // Insert our entry in the normal map
    st->entries.insert(MMapPair(name, node));
    // Remove all the copied entries with the same name
    st->copiedEntries.erase(name);
}

void Nest_symTabCopyEntries(SymTab* symTab, SymTab* otherSymTab)
{
    _SymTabImpl* st = (_SymTabImpl*) symTab;
    _SymTabImpl* other = (_SymTabImpl*) otherSymTab;

    for ( const auto& entry: other->entries )
    {
        // Insert the item into our set of copied entries, only if we don't find it in our normal set
        // This way, all the copied elements are shadowed by the normal entries
        auto it = st->entries.find(entry.first);
        if ( it == st->entries.end() )
        {
            st->copiedEntries.insert(entry);
        }
    }
    for ( const auto& entry: other->copiedEntries )
    {
        // Insert the item into our set of copied entries, only if we don't find it in our normal set
        // This way, all the copied elements are shadowed by the normal entries
        auto it = st->entries.find(entry.first);
        if ( it == st->entries.end() )
        {
            st->copiedEntries.insert(entry);
        }
    }

}

NodeVector Nest_symTabAllEntries(SymTab* symTab)
{
    _SymTabImpl* st = (_SymTabImpl*) symTab;

    NodeVector result;
    result.reserve(st->entries.size() + st->copiedEntries.size());
    transform(st->entries.begin(), st->entries.end(), back_inserter(result),
        boost::bind(&unordered_multimap<string, Node*>::value_type::second, _1) );
    transform(st->copiedEntries.begin(), st->copiedEntries.end(), back_inserter(result),
        boost::bind(&unordered_multimap<string, Node*>::value_type::second, _1) );
    return result;
}

NodeVector Nest_symTabLookupCurrent(SymTab* symTab, const char* name)
{
    _SymTabImpl* st = (_SymTabImpl*) symTab;

    auto range = st->entries.equal_range(name);
    NodeVector result;
    transform(range.first, range.second, back_inserter(result),
        boost::bind(&unordered_multimap<string, Node*>::value_type::second, _1) );
    if ( result.empty() )
    {
        auto range = st->copiedEntries.equal_range(name);
        transform(range.first, range.second, back_inserter(result),
            boost::bind(&unordered_multimap<string, Node*>::value_type::second, _1) );
    }
    return result;
}

NodeVector Nest_symTabLookup(SymTab* symTab, const char* name)
{
    _SymTabImpl* st = (_SymTabImpl*) symTab;

    NodeVector res = Nest_symTabLookupCurrent(symTab, name);
    return !res.empty() || !st->base.parent ? res : Nest_symTabLookup(st->base.parent, name);
}
