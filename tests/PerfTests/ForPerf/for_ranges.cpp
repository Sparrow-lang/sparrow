
#include <iostream>
#include <vector>

using namespace std;

template<typename T>
class NumericRange
{
    T begin;
    T end;

public:
    typedef T RetType;
    typedef T ValueType;

    NumericRange(T aStart, T aEnd) : begin(aStart), end(aEnd) {}

    bool isEmpty()                  { return begin == end; }
    T front()                       { return begin; }
    void popFront()                 { begin += 1; }
};

template<typename T>
NumericRange<T> mkNumericRange(T start, T end)
{
    return NumericRange<T>(start, end);
}
NumericRange<unsigned> mkNumericRange(unsigned start, unsigned end)
{
    return NumericRange<unsigned>(start, end);
}



unsigned long testAccumulate(unsigned n)
{
    unsigned long res = 0;
    auto r1 = mkNumericRange(0, n);
    for ( ; !r1.isEmpty(); r1.popFront() )
    {
        unsigned i = r1.front();

        auto r2 = mkNumericRange(0, n);
        for ( ; !r2.isEmpty(); r2.popFront() )
        {
            unsigned j = r2.front();
            res += i*j;
        }
    }
    return res;
}
int main(int argc, char** argv)
{
    if ( argc < 2 )
        return -1;
    unsigned n = atoi(argv[1]);

    unsigned long res = testAccumulate(n);
    cout << res << endl;

    return 0;
}