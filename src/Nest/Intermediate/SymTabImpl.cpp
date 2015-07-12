#include <StdInc.h>
#include "SymTabImpl.h"

#include <boost/bind.hpp>

using namespace Nest;

SymTabImpl::SymTabImpl(SymTab* parent, Node* node)
    : parent_(parent)
    , node_(node)
{
}

SymTabImpl::~SymTabImpl()
{
}

void SymTabImpl::enter(const string& name, Node* node)
{
    // Insert our entry in the normal map
    entries_.insert(make_pair(name, node));
    // Remove all the copied entries with the same name
    copiedEntries_.erase(name);
}

void SymTabImpl::copyEntries(SymTab* otherSymTab)
{
    SymTabImpl* other = static_cast<SymTabImpl*>(otherSymTab);
    for ( const auto& entry: other->entries_ )
    {
        // Insert the item into our set of copied entries, only if we don't find it in our normal set
        // This way, all the copied elements are shadowed by the normal entries
        auto it = entries_.find(entry.first);
        if ( it == entries_.end() )
        {
            copiedEntries_.insert(entry);
        }
    }
    for ( const auto& entry: other->copiedEntries_ )
    {
        // Insert the item into our set of copied entries, only if we don't find it in our normal set
        // This way, all the copied elements are shadowed by the normal entries
        auto it = entries_.find(entry.first);
        if ( it == entries_.end() )
        {
            copiedEntries_.insert(entry);
        }
    }
}

NodeVector SymTabImpl::allEntries() const
{
    NodeVector result;
    result.reserve(entries_.size() + copiedEntries_.size());
    transform(entries_.begin(), entries_.end(), back_inserter(result),
        boost::bind(&unordered_multimap<string, Node*>::value_type::second, _1) );
    transform(copiedEntries_.begin(), copiedEntries_.end(), back_inserter(result),
        boost::bind(&unordered_multimap<string, Node*>::value_type::second, _1) );
    return result;
}
DynNodeVector SymTabImpl::allEntriesDyn() const
{
    NodeVector r = allEntries();
    return move(reinterpret_cast<DynNodeVector&>(r));
}

NodeVector SymTabImpl::lookupCurrent(const string& name) const
{
    auto range = entries_.equal_range(name);
    NodeVector result;
    transform(range.first, range.second, back_inserter(result),
        boost::bind(&unordered_multimap<string, Node*>::value_type::second, _1) );
    if ( result.empty() )
    {
        auto range = copiedEntries_.equal_range(name);
        transform(range.first, range.second, back_inserter(result),
            boost::bind(&unordered_multimap<string, Node*>::value_type::second, _1) );
    }
    return result;
}
DynNodeVector SymTabImpl::lookupCurrentDyn(const string& name) const
{
    NodeVector r = lookupCurrent(name);
    return move(reinterpret_cast<DynNodeVector&>(r));
}

NodeVector SymTabImpl::lookup(const string& name) const
{
    NodeVector res = lookupCurrent(name);
    return !res.empty() || !parent_ ? res : parent_->lookup(name);
}
DynNodeVector SymTabImpl::lookupDyn(const string& name) const
{
    NodeVector r = lookup(name);
    return move(reinterpret_cast<DynNodeVector&>(r));
}

SymTab* SymTabImpl::parent() const
{
    return parent_;
}

Node* SymTabImpl::node() const
{
    return node_;
}

void SymTabImpl::dump(ostream& os) const
{
    dumpImpl(os, 0);
}

void SymTabImpl::dumpImpl(ostream& os, int nestLevel) const
{
    string spaces(nestLevel*4, ' ');
    os << spaces << "- Node: " << node_ << "\n";
    if ( !entries_.empty() )
    {
        os << spaces << "- Entries: " << "\n";
        for ( const auto& p: entries_ )
        {
            os << spaces << "    - " << p.first << " - " << p.second << "\n";
        }
    }
    if ( !copiedEntries_.empty() )
    {
        os << spaces << "- Copied entries: " << "\n";
        for ( const auto& p: copiedEntries_ )
        {
            os << spaces << "    - " << p.first << " - " << p.second << "\n";
        }
    }
    if ( parent_ )
    {
        if ( nestLevel < 2 )
        {
            os << spaces << "- Parent: \n";
            static_cast<SymTabImpl*>(parent_)->dumpImpl(os, nestLevel+1);
        }
        else
            os << spaces << "- Parent: ...\n";
    }
}
