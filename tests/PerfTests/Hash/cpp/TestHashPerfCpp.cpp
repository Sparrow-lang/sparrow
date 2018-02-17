
#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>

#define PAYLOAD_SIZE 16

struct Obj
{
    unsigned value;
    char payload[PAYLOAD_SIZE];
};

using std::cout;
using std::endl;
using namespace std;

namespace Spr
{
    namespace Impl
    {
        size_t bucketCounts[] = { 5, 11, 23, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741, 3221225473, 4294967291 };
    };

    template <typename T>
    class Ptr
    {
    public:
        typedef T ValueType;

        Ptr() : ptr(nullptr) {}
        Ptr(const Ptr& other) : ptr(other.ptr) {}
        Ptr(T& ref) : ptr(&ref) {}

        T& get()                    { return *ptr; }
        bool isNull()               { return !ptr; }
        bool isSet()                { return ptr; }

        void reset()                { ptr = nullptr; }
        void reset(T& ref)          { ptr = &ref; }
        void release()              { delete ptr; }

        // void swap(const Ptr& other)
        // {
        //     T& t = ptr;
        //     ptr = other.ptr;
        //     other.ptr = t;
        // }

        void operator =(const Ptr& other)   { ptr = other.ptr; }
        void operator =(T& ref)             { ptr = &ref; }

        bool operator ==(const Ptr& other) const  { return ptr == other.ptr; }
        bool operator !=(const Ptr& other) const  { return ptr != other.ptr; }
        bool operator < (const Ptr& other) const  { return ptr < other.ptr; }

        // fun >>(os: @OutStream) if isValid(os << #$ValueType)
        // {
        //     if ( ptr !== null )
        //         os << "Ptr(" << ptr << ")";
        //     else
        //         os << "Ptr(null)";
        //     os << flush;
        // }

    private:
        T* ptr;
    };

    template<typename T1, typename T2>
    class PairFirst
    {
    public:
        const T1& operator ()(const pair<T1, T2>& pair)
        {
            return pair.first;
        }
        T1& operator ()(pair<T1, T2>& pair)
        {
            return pair.first;
        }
    };

    // template<typename T1, typename T2>
    // class PairSecond
    // {
    // public:
    //     T2& operator ()(pair<T1, T2>& pair)
    //     {
    //         return pair.second;
    //     }
    // };

    template<typename T>
    class Equal
    {
    public:
        bool operator () (T x, T y) { return x == y; }
    };

    namespace HashImpl
    {
        static const size_t seed = 0xfadef00d;

        /// FNV hash generator
        // size_t doHash(const char* keyBegin, const char* keyEnd: StringRef, size_t start)
        // {
        //     size_t hash = start;
        //     for ( const char* p = keyBegin; p != keyEnd; ++p )
        //         hash = (hash ^ (unsigned char)(*p)) * 0x01000193;
        //     return hash;
        // }
        size_t doHash(size_t value, size_t start)
        {
            return (start ^ value) * 0x01000193;
        }
    }

    // size_t defaultHash(char arg)        = HashImpl.doHash(Byte(arg), HashImpl.seed);
    // size_t defaultHash(unsigned arg) { return HashImpl::doHash(arg, HashImpl::seed); }
    size_t defaultHash(unsigned arg) { return arg; }
    // size_t defaultHash(StringRef arg)   = HashImpl.doHash(arg, HashImpl.seed);
    // size_t defaultHash(@String arg)     = HashImpl.doHash(arg.asStringRef(), HashImpl.seed);

    template<typename T>
    class DefaultHashFunction
    {
    public:
        size_t operator ()(const T& arg) { return defaultHash(arg); }
    };

    //template <typename keyType, typename valueType, typename valueToKeyType, typename hashType, typename compareType>
    class HashTable
    {
    public:
        typedef unsigned KeyType;
        typedef pair<unsigned, Obj*> ValueType;
        typedef PairFirst<unsigned, Obj*> ValueToKeyType;
        typedef DefaultHashFunction<unsigned> HashType;
        typedef Equal<unsigned> CompareType;

        // typedef keyType KeyType;
        // typedef valueType ValueType;
        // typedef valueToKeyType ValueToKeyType;
        // typedef hashType HashType;
        // typedef compareType CompareType;

        class Node;
        // typedef Ptr<Node> NodePtr;
        typedef Node* NodePtr;
        typedef vector<NodePtr> BucketsType;

        class Node
        {
        public:
            NodePtr next;
            size_t hash;
            ValueType data;

            Node(NodePtr aNext, size_t aHash, const ValueType& aData)
                : next(aNext)
                , hash(aHash)
                , data(aData)
            {}

            // fun >> (os: @OutStream)
            // {
            //     os << data << " <" << mkStreamRefWrapper(this) << '>';
            // }
        };


        class GuardNode
        {
        public:
            NodePtr next;
        };

        class RangeType
        {
        public:
            typedef ValueType& RetType;

            NodePtr startElem, endElem;

            RangeType(): startElem(), endElem() {}
            RangeType(NodePtr start, NodePtr end)
                : startElem(start)
                , endElem(end)
            {}

            bool isEmpty() const    { return startElem == endElem; }
            RetType front()         { return startElem->data; }
            void popFront()         { startElem = startElem->next; }
        };

        HashTable()
            : buckets()
            , firstElem()
            , valToKey()
            , hash()
            , comp()
            , numElements(0)
        {
        }

        HashTable(size_t n, HashType ahash, CompareType acomp)
            : buckets()
            , firstElem()
            , valToKey()
            , hash(ahash)
            , comp(acomp)
            , numElements(0)
        {
            if ( n > 0)
                reserve(n);
        }

        // HashTable(range: Range, n: SizeType, ahash: HashType, acomp: CompareType)
        //     : buckets()
        //     , firstElem()
        //     , valToKey()
        //     , hash(ahash)
        //     , comp(acomp)
        //     , numElements(0)
        // {
        //     // reserve(max(n, rangeSize(range)));
        //     // for ( v = range )
        //     //     insertPlain(v);
        // }

        HashTable(const HashTable& other)
            : buckets(other.buckets)
            , firstElem(other.firstElem)
            , valToKey(other.valToKey)
            , hash(other.hash)
            , comp(other.comp)
            , numElements(other.numElements)
        {
            RangeType r = other.all();
            while ( !r.isEmpty() )
            {
                insertPlain(r.front());
                r.popFront();
            }
        }

        ~HashTable()
        {
            deleteElements();
        }

        // void swap(other: @HashTable)
        // {
        //     buckets.swap(other.buckets);
        //     swapTL(guard, other.guard);
        //     swapTL(hash, other.hash);
        //     swapTL(comp, other.comp);
        //     swapTL(numElements, other.numElements);

        //     // Update the buckets pointing to the guard node
        //     if ( numElements > 0 )
        //         buckets(guard.next->hash % buckets.size()) = guardNode();
        //     if ( other.numElements > 0 )
        //         other.buckets(other.guard.next->hash % other.buckets.size()) = other.guardNode();
        // }

        // fun hashFunction    = hash;
        // fun comparator      = comp;

        bool isEmpty()          { return numElements == 0; }
        size_t size()           { return numElements; }
        size_t bucketCount()    { return buckets.size(); }

        RangeType all() const   { return RangeType(firstElem, NodePtr()); }

        void reserve(size_t n)
        {
            // Resize only when we are big enough to re-hash
            if ( n <= buckets.size() /** 3*/ )
                return;

            // Determine the number of buckets
            int i = 0;
            while ( Impl::bucketCounts[i] < n )
                ++i;
            size_t numBuckets = Impl::bucketCounts[i];
            if ( numBuckets <= buckets.size() )
                return;

            // Re-create the buckets array
            buckets = BucketsType(numBuckets);

            // Recompute the proper buckets for our elements
            NodePtr p = firstElem;
            firstElem = nullptr;
            NodePtr next;
            for ( ; p != nullptr ; p = next )
            {
                next = p->next;
                insertNode(p);
            }
        }

        RangeType insert(const ValueType& value)
        {
            reserve(numElements + 1);
            return insertPlain(value);
        }

        // void insert(range: Range)
        // {
        //     reserve(numElements + rangeSize(range));
        //     var counter = 0;
        //     for ( v = range )
        //         insertPlain(v);
        // }

        void remove(const KeyType& key)
        {
            if ( isEmpty() )
                return;

            NodePtr node = findNode(key);
            if ( node == nullptr )
                return;

            NodePtr prev = findPrev(node);
            removeNode(prev, node);
        }

        void remove(const RangeType& range)
        {
            if ( range.isEmpty() )
                return;

            // Determine the element before the first element from the given range
            NodePtr prev = findPrev(range.startElem);

            NodePtr p = range.startElem;
            while ( p != range.endElem )
            {
                NodePtr next = p->next;
                removeNode(prev, p);
                p = next;
            }
        }

        bool contains(const KeyType& key)   { return findNode(key) != nullptr; }
        size_t count(const KeyType& key)    { return (findNode(key) != nullptr ? 1 : 0); }

        RangeType equalRange(const KeyType& key)
        {
            NodePtr p = findNode(key);
            return p != nullptr ? RangeType(p, p->next) : RangeType();
        }

        RangeType find(const KeyType& key)
        {
            NodePtr p = findNode(key);
            return RangeType(p, NodePtr());
            // return p != nullptr ? RangeType(p, NodePtr()) : RangeType();
        }

        void clear()
        {
            buckets.clear();
            numElements = 0;
            deleteElements();
        }

        // const HashTable& operator =(const HashTable& other)
        // {
        //     HashTable tmp = other;
        //     this.swap(tmp);
        //     return this;
        // }

        // bool operator ==(const HashTable& other)
        // {
        //     if ( !(hash == other.hash && comp == other.comp && numElements == other.size()) )
        //         return false;

        //     for ( v = all() )
        //     {
        //         if ( !other.contains(valToKey(v)) )
        //             return false;
        //     }

        //     return true;
        // }

        void dump()
        {
            cout << "HashTable, first=" << reinterpret_cast<void*>(firstElem) << ", size=" << numElements << endl;

            if ( isEmpty() )
                return;
            size_t numBuckets = buckets.size();
            NodePtr prev = guardNode();
            NodePtr p = prev->next;
            size_t prevBi = numBuckets+1;
            for ( ; p != nullptr ; p = p->next )
            {
                size_t bi = p->hash % numBuckets;
                if ( prev == guardNode() )
                {
                    cout << bi << ": <guard> | ";
                    prevBi = bi;
                }
                else if ( bi != prevBi )
                {
                    cout << endl << bi << ": " << prevBi << '-' << prev->data.first << " | ";
                    prevBi = bi;
                }
                else
                    cout << ' ' << prevBi << '-' << prev->data.first;
                prev = p;
                //cout << '(' << p.get() << ')';
            }
            cout << ' ' << prevBi << '-' << prev->data.first;
            cout << endl;
        }

    // private:
        BucketsType buckets;
        // GuardNode guard;
        NodePtr firstElem;
        size_t numElements;
        ValueToKeyType valToKey;
        HashType hash;
        CompareType comp;

        // NodePtr guardNode()         { return NodePtr(*reinterpret_cast<Node*>(&guard)); }
        NodePtr guardNode()         { return NodePtr(reinterpret_cast<NodePtr*>(&firstElem)); }
        NodePtr getFirstElem()      { return firstElem; }

        NodePtr findNode(const KeyType& key)
        {
            // if ( isEmpty() )
            //     return NodePtr();

            // return findNodeInBucket(hash(key) % buckets.size(), key);
            size_t keyHash = hash(key);
            size_t numBuckets = buckets.size();
            // cout << "Finding elem " << key << " in hash map:" << endl;
            // dump();
            if ( numBuckets != 0 )
            {
                size_t bi = keyHash % numBuckets;
                NodePtr p = buckets[bi];
                if ( p != nullptr )
                {
                    p = p->next;
                    while ( p != nullptr && (p->hash % numBuckets) == bi )
                    {
                        if ( comp(key, valToKey(p->data)) )
                            return p;
                        p = p->next;
                    }
                }
            }
            return NodePtr();
        }

        NodePtr findNodeInBucket(size_t bi, const KeyType& key)
        {
            NodePtr p = buckets[bi];
            if ( p == nullptr )
                return NodePtr();
            // the first element belongs to the previous bucket, skip one element
            NodePtr next = p->next;
            size_t numBuckets = buckets.size();
            //cout << "Start searching for " << key << " at p=" << p.get() << ", next=" << next << endl;
            while ( next != nullptr && (next->hash % numBuckets == bi) )
            {
                //cout << "p=" << p.get() << ", next=" << next << endl;
                if ( comp(key, valToKey(next->data)) )
                    return next;
                p = next;
                next = p->next;
            }
            return NodePtr();
        }

        NodePtr findPrev(NodePtr node)
        {
            size_t bi = node->hash % buckets.size();
            NodePtr p = buckets[bi];
            while ( p->next != node )
                p = p->next;
            return p;
        }

        RangeType insertPlain(const ValueType& value)
        {
            auto key = valToKey(value);
            auto h = hash(key);
            size_t bi = h % buckets.size();

            // Insert only if the element is not found in the hash
            NodePtr node = findNodeInBucket(bi, key);
            if ( node == nullptr )
            {
                node = new Node(NodePtr(), h, value);
                ++numElements;
                insertNode(node);
            }
            //cout << "After insert(" << value << "): " << this << endl;
            return RangeType(node, NodePtr());
        }

        void insertNode(NodePtr node)
        {
            // If there is no element in the bucket, put this in the bucket, and chain the element at the beginning of the global list
            // Otherwise, put this after the first element in the bucket
            size_t numBuckets = buckets.size();
            size_t bi = node->hash % numBuckets;
            if ( buckets[bi] == nullptr )
            {
                node->next = firstElem;
                firstElem = node;
                buckets[bi] = guardNode();
                // fix the bucket of the old start
                if ( node->next != nullptr )
                    buckets[node->next->hash % numBuckets] = node;
            }
            else
            {
                node->next = buckets[bi]->next;
                buckets[bi]->next = node;
            }
        }

        void removeNode(NodePtr prev, NodePtr node)
        {
            size_t numBuckets = buckets.size();
            size_t bi = node->hash % numBuckets;
            NodePtr next = node->next;

            // If this is the only proper node for this bucket, clear the bucket
            if ( prev == guardNode() || bi != (prev->hash % numBuckets) ) // this is second
            {
                if ( next == nullptr || bi != (next->hash % numBuckets) ) // next is not in this bucket
                    buckets[bi] = nullptr;
            }

            // If the next node is in another bucket, update the first node from that bucket
            if ( next != nullptr )
            {
                size_t nextBi = next->hash % numBuckets;
                if ( bi != nextBi )
                    buckets[nextBi] = prev;
            }

            // Now remove the node
            prev->next = next;
            delete node;
            --numElements;
        }

        void deleteElements()
        {
            NodePtr p = firstElem;
            NodePtr next;
            for ( ; p != nullptr ; p = next )
            {
                next = p->next;
                delete p;
            }
            firstElem = NodePtr();
        }
    };


    //template<typename keyType, typename dataType, typename hashType, typename compareType>
    class HashMap
    {
    public:
        typedef unsigned KeyType;
        typedef Obj* DataType;
        typedef DefaultHashFunction<unsigned> HashType;
        typedef Equal<unsigned> CompareType;
        typedef pair<unsigned, Obj*> ValueType;
        typedef HashTable ImplTable;
        typedef typename ImplTable::RangeType RangeType;

        // typedef keyType KeyType;
        // typedef dataType DataType;
        // typedef hashType HashType;
        // typedef compareType CompareType;
        // typedef pair<KeyType, DataType> ValueType;
        // typedef HashTable<keyType, ValueType, PairFirst<KeyType, DataType>, hashType, compareType> ImplTable;
        // typedef typename ImplTable::RangeType RangeType;

    // private:
        ImplTable hashTable;

    public:

        HashMap(): hashTable(0, HashType(), CompareType()) {}
        HashMap(size_t n): hashTable(n, HashType(), CompareType()) {}
        // fun ctor(size_t n, hash: HashType)                       { hashTable.ctor(n, hash, CompareType()); }
        // fun ctor(size_t n, hash: HashType, comp: CompareType)    { hashTable.ctor(n, hash, comp); }
        // fun ctor(range: Range)                                      { hashTable.ctor(range, 0, HashType(), CompareType()); }
        // fun ctor(range: Range, n: SizeType)                         { hashTable.ctor(range, n, HashType(), CompareType()); }
        // fun ctor(range: Range, n: SizeType, hash: HashType)         { hashTable.ctor(range, n, hash, CompareType()); }
        // fun ctor(range: Range, n: SizeType, hash: HashType, comp: CompareType)  { hashTable.ctor(range, n, hash, comp); }
        HashMap(const HashMap& other): hashTable(other.hashTable) {}

        // fun hashFunction                            = hashTable.hash;
        // fun comparator                              = hashTable.comp;

        size_t size()                               { return hashTable.size(); }
        bool isEmpty()                              { return hashTable.isEmpty(); }
        size_t bucketCount()                        { return hashTable.bucketCount(); }
        void reserve(size_t n)                      { hashTable.reserve(n); }

        RangeType all() const                       { return hashTable.all(); }
        // fun keys                                    = transform(all(), PairFirst(ValueType)());
        // fun values                                  = transform(all(), PairSecond(ValueType)());

        RangeType insert(const KeyType& key, const DataType& data)  { return hashTable.insert(ValueType(key, data)); }
        RangeType insert(const ValueType& value)    { return hashTable.insert(value); }
        // void insert(range: Range)                   { hashTable.insert(range); }
        void remove(const KeyType& key)             { hashTable.remove(key); }
        void remove(RangeType range)                { hashTable.remove(range); }

        bool contains(const KeyType& key)           { return hashTable.contains(key); }
        size_t count(const KeyType& key)            { return hashTable.count(key); }
        RangeType equalRange(const KeyType& key)    { return hashTable.equalRange(key); }
        RangeType find(const KeyType& key)          { return hashTable.find(key); }

        DataType& operator()(const KeyType& key)    { return at(key); }
        DataType& at(const KeyType& key)
        {
            RangeType r = find(key);
            if ( r.isEmpty() )
            {
                DataType data;
                r = hashTable.insert(ValueType(key, data));
            }
            return r.front().second;
        }

        void clear()                                 { hashTable.clear(); }

        // void swap(const HashMap& other)              { hashTable.swap(other.hashTable); }

        // fun >> (os: @OutStream)                     { hashTable.>>(os); }
    };
};


std::vector<Obj> objects;
std::vector<unsigned> insertKeys;
std::vector<unsigned> searchKeys;

class Timer
{
public:
    Timer(const char* desc, unsigned n)
        : desc_(desc)
        , n_(n)
        , startTime_(clock())
    {}

    ~Timer()
    {
        double t = elapsed();
        double t_ns = t * 1000000.0 / double(n_);
        cout << desc_ << ":\t" << t << ", " << t_ns << " ns" << endl;
    }

    double elapsed()            { return double(clock() - startTime_) / 1000.0; }

    void restart()              { startTime_ = clock(); }

private:
    const char* desc_;
    unsigned n_;
    clock_t startTime_;
};


template <typename T>
void testInsert(T& bag, unsigned n)
{
    for ( unsigned i=0; i<n; ++i )
    {
        // The element to insert
        unsigned key = insertKeys[i];
        Obj* obj = &objects[i];
        obj->value = key;

        // Insert it
        bag[key] = obj;
    }
}

template <typename T>
void testSprInsert(T& bag, unsigned n)
{
    for ( unsigned i=0; i<n; ++i )
    {
        // The element to insert
        unsigned key = insertKeys[i];
        Obj* obj = &objects[i];
        obj->value = key;

        // Insert it
        bag(key) = obj;
    }
}

template <typename T>
void testChange(T& bag, unsigned n)
{
    for ( unsigned i=0; i<n; ++i )
    {
        // Search the element
        unsigned key = searchKeys[i];
        typename T::iterator it = bag.find(key);
        if ( it == bag.end() )
        {
            cout << "FAIL: Cannot find element " << key << " in map!" << endl;
            return;
        }

        // Remove it from the map
        Obj* obj = it->second;
        bag.erase(it);

        // Reinsert with a new key
        key = insertKeys[i] + 1;
        obj->value = key;
        bag[key] = obj;
    }
}

template <typename T>
void testSprChange(T& bag, unsigned n)
{
    for ( unsigned i=0; i<n; ++i )
    {
        // Search the element
        unsigned key = searchKeys[i];
        auto r = bag.equalRange(key);
        if ( r.isEmpty() )
        {
            cout << "FAIL: Cannot find element " << key << " in map!" << endl;
            exit(-1);
            return;
        }

        // Remove it from the map
        auto obj = r.front().second;
        bag.remove(r);

        // Reinsert with a new key
        key = insertKeys[i] + 1;
        obj->value = key;
        bag(key) = obj;
    }
}

template <typename T>
__declspec(noinline)
void testHit(T& bag, unsigned n)
{
    for ( unsigned i=0; i<n; ++i )
    {
        // Search the element
        unsigned key = searchKeys[i] + 1;
        typename T::iterator it = bag.find(key);
        if ( it == bag.end() )
        {
            exit(-1);
            // cout << "FAIL: Cannot find element " << key << " in map!" << endl;
            // return;
        }

        // Ensure that we have the correct element
        Obj* obj = it->second;
        if ( obj->value != key )
        {
            exit(-2);
            // cout << "FAIL: Element with key " << key << " has an invalid value: " << obj->value << endl;
            // return;
        }
    }
}

template <typename T>
__declspec(noinline)
void testSprHit(T& bag, unsigned n)
{
    for ( unsigned i=0; i<n; ++i )
    {
        // Search the element
        unsigned key = searchKeys[i] + 1;
        // auto r = bag.find(key);
        auto n = bag.hashTable.findNode(key);
        // auto r = bag.equalRange(key);
        if ( n == nullptr )
        // if ( r.isEmpty() )
        {
            exit(-1);
            // cout << "FAIL: Cannot find element " << key << " in map!" << endl;
            // return;
        }

        // Ensure that we have the correct element
        auto obj = n->data.second;
        // auto obj = r.front().second;
        if ( obj->value != key )
        {
            exit(-2);
            // cout << "FAIL: Element with key " << key << " has an invalid value: " << obj->value << endl;
            // return;
        }
    }
}

template <typename T>
void testMiss(T& bag, unsigned n)
{
    for ( unsigned i=0; i<n; ++i )
    {
        // Search the element
        // As all elements are shifted by one, we will find nothing
        unsigned key = searchKeys[i];
        typename T::iterator it = bag.find(key);
        if ( it != bag.end() )
        {
            cout << "FAIL: Element " << key << " shouldn't be in the map!" << endl;
            return;
        }
    }
}

template <typename T>
void testSprMiss(T& bag, unsigned n)
{
    for ( unsigned i=0; i<n; ++i )
    {
        // Search the element
        // As all elements are shifted by one, we will find nothing
        unsigned key = searchKeys[i];
        auto r = bag.equalRange(key);
        if ( !r.isEmpty() )
        {
            cout << "FAIL: Element " << key << " shouldn't be in the map!" << endl;
            return;
        }
    }
}

template <typename T>
void testRemove(T& bag, unsigned n)
{
    for ( unsigned i=0; i<n; ++i )
    {
        // Search the element
        unsigned key = searchKeys[i] + 1;
        typename T::iterator it = bag.find(key);
        if ( it == bag.end() )
        {
            cout << "FAIL: Cannot find element " << key << " in map!" << endl;
            return;
        }

        // Remove it from the map
        Obj* obj = it->second;
        bag.erase(it);

        // Ensure that we have the correct element
        if ( obj->value != key )
        {
            cout << "FAIL: Element with key " << key << " has an invalid value: " << obj->value << endl;
            return;
        }
    }
}

template <typename T>
void testSprRemove(T& bag, unsigned n)
{
    for ( unsigned i=0; i<n; ++i )
    {
        // Search the element
        unsigned key = searchKeys[i] + 1;
        auto r = bag.equalRange(key);
        if ( r.isEmpty() )
        {
            cout << "FAIL: Cannot find element " << key << " in map!" << endl;
            return;
        }

        auto obj = r.front().second;

        // Remove it
        bag.remove(r);

        // Ensure that we have the correct element
        if ( obj->value != key )
        {
            cout << "FAIL: Element with key " << key << " has an invalid value: " << obj->value << endl;
            return;
        }
    }
}


typedef std::map<unsigned, Obj*> TestStdMap;
typedef std::unordered_map<unsigned, Obj*> TestStdUnorderedMap;

// typedef Spr::HashMap<unsigned, Obj*, Spr::DefaultHashFunction<unsigned>, Spr::Equal<unsigned> > TestSprMap;
typedef Spr::HashMap TestSprMap;

void checkHashSizes(unsigned n)
{
    TestStdUnorderedMap theMap;
    size_t prevBucketCount = theMap.bucket_count();
    cout << "0 -> " << prevBucketCount << "\n";
    for ( unsigned i=0; i<n; ++i )
    {
        theMap[i] = nullptr;
        size_t bc = theMap.bucket_count();
        if ( bc != prevBucketCount )
        {
            cout << i+1 << " -> " << bc << "\n";
            prevBucketCount = bc;
        }
    }
    cout << endl;
}

void checkVectorCapacities(unsigned n)
{
    vector<unsigned> vec;
    size_t prevCapacity = vec.capacity();
    cout << "0 -> " << prevCapacity << "\n";
    for ( unsigned i=0; i<n; ++i )
    {
        vec.push_back(i);
        size_t capacity = vec.capacity();
        if ( capacity != prevCapacity )
        {
            cout << i+1 << " -> " << capacity << "\n";
            prevCapacity = capacity;
        }
    }
    cout << endl;
}

int main(int argc, char** argv)
{
    const unsigned n = 10000000;
    // const unsigned n = 10;

    // Preallocate the objects
    objects.resize(n);

    // Initialize the keys
    insertKeys.resize(n);
    searchKeys.resize(n);
    for ( int i =0; i<n; ++i )
    {
        insertKeys[i] = 0x80000000 + i*2;
        searchKeys[i] = 0x80000000 + i*2;
    }
    // TODO: random shuffling

    checkHashSizes(n);
    checkVectorCapacities(n);

    TestStdUnorderedMap bagUnorderedMap;
    {
        Timer timer("unsorted_map Insert", n);
        testInsert(bagUnorderedMap, n);
    }
    // system("sleep 1");
    {
        Timer timer("unsorted_map Change", n);
        testChange(bagUnorderedMap, n);
    }
    // system("sleep 1");
    // cout << "Bucket count: " << bagUnorderedMap.bucket_count() << endl;
    {
        Timer timer("unsorted_map Hit   ", n);
        testHit(bagUnorderedMap, n);
    }
    {
        Timer timer("unsorted_map Miss  ", n);
        testMiss(bagUnorderedMap, n);
    }
    {
        Timer timer("unsorted_map Remove", n);
        testRemove(bagUnorderedMap, n);
    }

    cout << endl;
    // system("sleep 2");

    TestSprMap bagSprMap;
    {
        Timer timer("Spr HashMap Insert", n);
        testSprInsert(bagSprMap, n);
    }
    // system("sleep 1");
    {
        Timer timer("Spr HashMap Change", n);
        testSprChange(bagSprMap, n);
    }
    // system("sleep 1");
    // cout << "Bucket count: " << bagSprMap.bucketCount() << endl;
    {
        Timer timer("Spr HashMap Hit   ", n);
        testSprHit(bagSprMap, n);
    }
    {
        Timer timer("Spr HashMap Miss  ", n);
        testSprMiss(bagSprMap, n);
    }
    {
        Timer timer("Spr HashMap Remove", n);
        testSprRemove(bagSprMap, n);
    }

    return 0;
}