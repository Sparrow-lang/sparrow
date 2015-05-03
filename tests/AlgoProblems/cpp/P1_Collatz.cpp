// P1_Collatz.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TestUtil.h"

using namespace std;

typedef unsigned long long uint64;

static const size_t vecSize = 1 << 28;

struct Result
{
    Result() : pos(1), len(1) {}
    Result(uint64 p, int l) : pos(p), len(l) {}

    uint64 pos;
    int len;
};

inline void updateResult(Result& res, uint64 curPos, int curLen)
{
    if ( curLen >= res.len )
    {
//         res = Result(curPos, curLen);
        res.pos = curPos;
        res.len = curLen;
    }
}

inline void updateResultAndPrev(Result& res, Result& prevRes, uint64 curPos, int curLen)
{
    if ( curLen >= prevRes.len )
    {
        if ( curLen >= res.len )
        {
//             prevRes.pos = res.pos;
//             prevRes.len = res.len;
            prevRes = res;
            res = Result(curPos, curLen);
//             res.pos = curPos;
//             res.len = curLen;
        }
        else
        {
//             prevRes = Result(curPos, curLen);
            prevRes.pos = curPos;
            prevRes.len = curLen;
        }
    }

}


inline int lengthStartingWith(uint64 n)
{
    int res = 1;
    while ( n != 1 )
    {
        if ( n % 2 == 0 )
            n = n/2;
        else
            n = 3*n + 1;
        ++res;
    }
    return res;
}


void buildCacheVector(uint64 size, vector<short>& v, Result& res)
{
    for ( uint64 i=1; i<=size; ++i )
    {
        int len = 1;
        uint64 n = i;
        uint64 lastCached = min(i, size);
        while ( n != 1 )
        {
            if ( n % 2 == 0 )
            {
                n = n/2;
                if ( n<lastCached )
                {
                    len += v[size_t(n)];
                    break;
                }
            }
            else
                n = 3*n + 1;

            ++len;
        }
        v[size_t(i)] = len;

        updateResult(res, i, len);
    }
}

void searchAfterCache(uint64 limit, uint64 size, vector<short>& v, Result& res)
{
    for ( uint64 i=size; i<=limit; ++i )
    {
        int len = 1;
        uint64 n = i;
        while ( n != 1 )
        {
            if ( n % 2 == 0 )
            {
                n = n/2;
                if ( n<size )
                {
                    len += v[size_t(n)];
                    break;
                }
            }
            else
                n = 3*n + 1;

            ++len;
        }
        updateResult(res, i, len);
    }
}

#define ds2_getIdx(i)   size_t(i < size ? i : i-size)

void updateNextLevelCache(uint64 limit, uint64 size, vector<short>& v, Result& res)
{
    uint64 size2 = size*2;
    if ( limit > size2 )
        limit = size2;
    for ( uint64 i=size; i<=limit; ++i )
    {
        int len = 1;
        uint64 n = i;
        while ( n != 1 )
        {
            if ( n % 2 == 0 )
            {
                n = n/2;
                if ( n<i )
                {
                    len += v[ds2_getIdx(n)];
                    break;
                }
            }
            else
                n = 3*n + 1;

            ++len;
        }
        v[size_t(i-size)] = len;

        updateResult(res, i, len);
    }
}

void searchAfterNextLevelCache(uint64 start, uint64 limit, uint64 size, vector<short>& v, Result& res)
{
    const uint64 size2 = 2*size;
    for ( uint64 i=start; i<=limit; ++i )
    {
        int len = 1;
        uint64 n = i;
        while ( n != 1 )
        {
            if ( n % 2 == 0 )
            {
                n = n/2;
                if ( n<size2 )
                {
                    len += v[size_t(n - size)];
                    break;
                }
            }
            else
                n = 3*n + 1;

            ++len;
        }
        updateResult(res, i, len);
    }
}

void searchAfterNextLevelCacheWithCutoff(uint64 start, uint64 limit, uint64 size, vector<short>& v, Result& res, Result& prevRes)
{
    const uint64 size2 = 2*size;
    for ( uint64 i=start; i<=limit; ++i )
    {
        int len = 1;
        uint64 n = i;
        while ( n != 1 )
        {
            if ( n % 2 == 0 )
            {
                n = n/2;
                if ( n<size2 )
                {
                    len += v[size_t(n - size)];
                    break;
                }
                if ( n < i && len+prevRes.len < res.len )
                    break;
            }
            else
                n = 3*n + 1;

            ++len;
        }
        updateResultAndPrev(res, prevRes, i, len);
    }
}



uint64 naiveSearch(uint64 limit)
{
    Result res;
    for ( uint64 i=1; i<=limit; ++i )
    {
        int len = lengthStartingWith(i);
        updateResult(res, i, len);
    }
    return res.pos;
}

uint64 cutoffSearch(uint64 limit)
{
    Result res;
    Result prevRes;
    for ( uint64 i=1; i<=limit; ++i )
    {
        int len = 1;
        uint64 n = i;
        while ( n != 1 )
        {
            if ( n % 2 == 0 )
            {
                n = n/2;
                if ( n < i && len+prevRes.len < res.len )
                    break;
            }
            else
                n = 3*n + 1;

            ++len;
        }
        updateResultAndPrev(res, prevRes, i, len);
    }
    return res.pos;
}

uint64 dynamicSearch(uint64 limit)
{
    Result res;
    const size_t size = limit > vecSize ? vecSize : (size_t) limit;
    vector<short> v(size, 0);
    buildCacheVector((uint64) size, v, res);
    searchAfterCache(limit, size, v, res);
    return res.pos;
}

uint64 dynamicSearch2(uint64 limit)
{
    const size_t size = limit > vecSize ? vecSize : (size_t) limit;
    const size_t size2 = 2*size;
    if ( limit < size2 )
        return dynamicSearch(limit);
    vector<short> v(size, 0);
    Result res;
    buildCacheVector((uint64) size, v, res);
    updateNextLevelCache(limit, (uint64) size, v, res);
    searchAfterNextLevelCache(size*2, limit, (uint64) size, v, res);
    return res.pos;
}

uint64 dynamicSearch2WithCutoff(uint64 limit)
{
    const size_t size = limit > vecSize ? vecSize : (size_t) limit;
    const size_t size2 = 2*size;
    if ( limit < size2 )
        return dynamicSearch(limit);
    vector<short> v(size, 0);
    Result res;
    Result prevRes;
    buildCacheVector((uint64) size, v, res);
    updateNextLevelCache(limit, (uint64) size, v, res);
    searchAfterNextLevelCacheWithCutoff(size*2, limit, (uint64) size, v, res, prevRes);
    return res.pos;
}

#ifdef USE_TBB

using namespace tbb;

struct BasicTbbResult
{
    void join(BasicTbbResult& rhs)
    {
        if ( (res.len < rhs.res.len)
            || (res.len == rhs.res.len && res.pos < rhs.res.pos) )
        {
            res = rhs.res;
        }
    }

    Result res;
};

struct BasicTbbResult2
{
    void join(BasicTbbResult2& rhs)
    {
        if ( (res.len < rhs.res.len)
            || (res.len == rhs.res.len && res.pos < rhs.res.pos) )
        {
            res = rhs.res;
            prevRes = rhs.prevRes;
        }
    }

    Result res;
    Result prevRes;
};

struct TbbCutoffSearchRes : BasicTbbResult2
{
    TbbCutoffSearchRes() {}
    TbbCutoffSearchRes(const TbbCutoffSearchRes& other, split) {}

    void operator() (const blocked_range<uint64>& r)
    {
        for ( uint64 i=r.begin(); i<r.end(); ++i )
        {
            int len = 1;
            uint64 n = i;
            while ( n != 1 )
            {
                if ( n % 2 == 0 )
                {
                    n = n/2;
                    if ( n < i && len+prevRes.len < res.len )
                        break;
                }
                else
                    n = 3*n + 1;

                ++len;
            }
            updateResultAndPrev(res, prevRes, i, len);
        }
    }

    void join(TbbCutoffSearchRes& rhs)
    {
        if ( (res.len < rhs.res.len)
            || (res.len == rhs.res.len && res.pos < rhs.res.pos) )
        {
            res = rhs.res;
            prevRes = rhs.prevRes;
        }
    }

    Result res;
    Result prevRes;
};

struct TbbBuildCacheVector : BasicTbbResult
{
    TbbBuildCacheVector(vector<short>& vv): v(vv) {}
    TbbBuildCacheVector(const TbbBuildCacheVector& other, split) : v(other.v) {}

    void operator() (const blocked_range<uint64>& r)
    {
        const size_t size = v.size();
        for ( uint64 i=r.begin(); i<r.end(); ++i )
        {
            int len = 1;
            uint64 n = i;
            while ( n != 1 )
            {
                if ( n % 2 == 0 )
                {
                    n = n/2;
                    if ( n<i )
                    {
                        const short len1 = -v[size_t(n)];
                        if ( len1 > 0 )
                        {
                            len += len1;
                            break;
                        }
                    }
                }
                else
                    n = 3*n + 1;

                ++len;
            }
            v[size_t(i)] = -len;

            updateResult(res, i, len);
        }
    }

    vector<short>& v;
};

struct TbbSearchAfterCache : BasicTbbResult
{
    TbbSearchAfterCache(TbbBuildCacheVector& base): v(base.v) { res = base.res; }
    TbbSearchAfterCache(const TbbSearchAfterCache& other, split) : v(other.v) {}

    void operator() (const blocked_range<uint64>& r)
    {
        const size_t size = v.size();
        for ( uint64 i=r.begin(); i<r.end(); ++i )
        {
            int len = 1;
            uint64 n = i;
            while ( n != 1 )
            {
                if ( n % 2 == 0 )
                {
                    n = n/2;
                    if ( n<size )
                    {
                        // TODO: Remove if here
                        const short len1 = -v[size_t(n)];
                        if ( len1 > 0 )
                        {
                            len += len1;
                            break;
                        }
                    }
                }
                else
                    n = 3*n + 1;

                ++len;
            }
            updateResult(res, i, len);
        }
    }

    vector<short>& v;
};

struct TbbUpdateNextLevelCache : BasicTbbResult
{
    TbbUpdateNextLevelCache(TbbBuildCacheVector& base): v(base.v) { res = base.res; }
    TbbUpdateNextLevelCache(const TbbUpdateNextLevelCache& other, split) : v(other.v) {}

    void operator() (const blocked_range<uint64>& r)
    {
        const size_t size = v.size();
        uint64 size2 = size*2;
        for ( uint64 i=r.begin(); i<r.end(); ++i )
        {
            int len = 1;
            uint64 n = i;
            while ( n != 1 )
            {
                if ( n % 2 == 0 )
                {
                    n = n/2;
                    if ( n<size )
                    {
                        const short len1 = -v[size_t(n)];
                        if ( len1 > 0 )
                        {
                            len += len1;
                            break;
                        }
                    }
                    else if ( n<size2 )
                    {
                        const short len1 = v[size_t(n - size)];
                        if ( len1 > 0 )
                        {
                            len += len1;
                            break;
                        }
                    }
                }
                else
                    n = 3*n + 1;

                ++len;
            }
            v[size_t(i - size)] = len;
            updateResult(res, i, len);
        }
    }

    vector<short>& v;
};

struct TbbSearchAfterNextLevelCache : BasicTbbResult
{
    TbbSearchAfterNextLevelCache(TbbUpdateNextLevelCache& base): v(base.v) { res = base.res; }
    TbbSearchAfterNextLevelCache(const TbbSearchAfterNextLevelCache& other, split) : v(other.v) {}

    void operator() (const blocked_range<uint64>& r)
    {
        const size_t size = v.size();
        const size_t size2 = 2*size;
        for ( uint64 i=r.begin(); i<r.end(); ++i )
        {
            int len = 1;
            uint64 n = i;
            while ( n != 1 )
            {
                if ( n % 2 == 0 )
                {
                    n = n/2;
                    if ( n<size2 )
                    {
                        len += v[size_t(n - size)];
                        break;
                    }
                }
                else
                    n = 3*n + 1;

                ++len;
            }
            updateResult(res, i, len);
        }
    }

    vector<short>& v;
};


uint64 tbbCutoffSearch(uint64 limit)
{
    TbbCutoffSearchRes result;
    parallel_reduce(blocked_range<uint64>(1, limit+1), result);
    return result.res.pos;
}

uint64 tbbDynamicSearch(uint64 limit)
{
    const size_t size = limit > vecSize ? vecSize : (size_t) limit;
    vector<short> v(size, 0);
    TbbBuildCacheVector r1(v);
    parallel_reduce(blocked_range<uint64>(1, size), r1);
    TbbSearchAfterCache r2(r1);
    parallel_reduce(blocked_range<uint64>(size, limit), r2);
    return r2.res.pos;
}

uint64 tbbDynamicSearch2(uint64 limit)
{
    const size_t size = limit > vecSize ? vecSize : (size_t) limit;
    vector<short> v(size, 0);
    TbbBuildCacheVector r1(v);
    parallel_reduce(blocked_range<uint64>(1, size), r1);

    uint64 size2 = size*2;
    if ( limit <= size2 )
    {
        TbbSearchAfterCache r2(r1);
        parallel_reduce(blocked_range<uint64>(size, limit), r2);
        return r2.res.pos;
    }
    TbbUpdateNextLevelCache r2(r1);
    parallel_reduce(blocked_range<uint64>(size, size2), r2);
    TbbSearchAfterNextLevelCache r3(r2);
    parallel_reduce(blocked_range<uint64>(size2, limit), r3);
    return r3.res.pos;
}
#endif

uint64 bestSearchAlgo(uint64 limit)
{
#ifdef USE_TBB
    return tbbDynamicSearch2(limit);
#else
    return dynamicSearch2WithCutoff(limit);
#endif
}


template <typename Fun>
class TimerTestFn
{
public:
    explicit TimerTestFn(Fun testFun, uint64 n, uint64& res) : testFun_(testFun), n_(n), res_(res) {}
    void operator()()
    {
        res_ = testFun_(n_);
    }

private:
    Fun testFun_;
    uint64 n_;
    uint64& res_;
};

template <typename T>
TimerTestFn<T> createTimerTestFun(const T& testFun, uint64 n, uint64& res)
{
    return TimerTestFn<T>(testFun, n, res);
}

void checkResult(const char* test, uint64 expected, uint64 res)
{
    if ( res != 0 && res != expected )
        cout << "ALGORITHM BUG on test " << test << ": expected:" << expected << ", got:" << res << endl;
}

void testSpeed(uint64 start, uint64 end, uint64 step, bool enableSlowTests = true)
{
    //DEFINE_STREAM("speed.csv");
    ostream& f = cout;

    f << "count, best";
    if ( enableSlowTests )
        f << ", naive, cutoff";
    f << ", dynamic, dynamic2, dynamic2cutoff";
#ifdef USE_TBB
    f << ", tbb_cutoff, tbb_dynamic, tbb_dynamic_cutoff, tbb_dynamic2, tbb_dynamic2cutoff";
#endif
    f << endl;

    for ( uint64 n=start; n<=end; n+=step )
    {
        // Variables used to hold the results
        uint64 r1, r2, r3, r4, r5, r6;
        uint64 pr1, pr2, pr3, pr4, pr5, pr6;
        r1 = r2 = r3 = r4 = r5 = r6 = 0;
        pr1 = pr2 = pr3 = pr4 = pr5 = pr6 = 0;

        // Call the different search methods, measure the time and output the time
        f << n;
        f << ", " << measureTime(createTimerTestFun(&bestSearchAlgo, n, r1), 1, 0.0);
        f << "   ";
        if ( enableSlowTests )
        {
            f << ", " << measureTime(createTimerTestFun(&naiveSearch, n, r2), 1, 0.0);
            f << ", " << measureTime(createTimerTestFun(&cutoffSearch, n, r3), 1, 0.0);
        }
//         f << ", " << measureTime(createTimerTestFun(&dynamicSearch, n, r4), 1, 0.0);
//         f << ", " << measureTime(createTimerTestFun(&dynamicSearch2, n, r5), 1, 0.0);
//         f << ", " << measureTime(createTimerTestFun(&dynamicSearch2WithCutoff, n, r6), 1, 0.0);
#ifdef USE_TBB
        f << "   ";
//         f << ", " << measureTime(createTimerTestFun(&tbbCutoffSearch, n, pr1), 1, 0.0);
//         f << ", " << measureTime(createTimerTestFun(&tbbDynamicSearch, n, pr2), 1, 0.0);
//         f << ", " << measureTime(createTimerTestFun(&tbbDynamicSearch2, n, pr3), 1, 0.0);
#endif
        f << endl;

        // Check the results of the computations
        checkResult("2", r1, r2);
        checkResult("3", r1, r3);
        checkResult("4", r1, r4);
        checkResult("5", r1, r5);
        checkResult("6", r1, r6);
        checkResult("tbb 1", r1, pr1);
        checkResult("tbb 2", r1, pr2);
        checkResult("tbb 3", r1, pr3);
        checkResult("tbb 4", r1, pr4);
        checkResult("tbb 5", r1, pr5);
        checkResult("tbb 6", r1, pr6);
    }
}




int main(int argc, const char** argv)
{
    //const uint64 nt = 10000000;
//     const uint64 nt = uint64(1) << 20;
//     testSpeed(nt, nt, 1, false);
//     cout << "done." << endl;
    //testSpeed(vecSize/2, vecSize*8, vecSize/4, false);

    if ( argc < 2 )
        return -1;
    uint64 n = 0;
    //n = atol(argv[1]);
    n = _atoi64(argv[1]);
    //cout << n << endl;
    uint64 res = bestSearchAlgo(n);
    cout << res << endl;
	return 0;
}

