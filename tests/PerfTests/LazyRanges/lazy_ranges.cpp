
#include <iostream>
#include <math.h>

using namespace std;

unsigned long long nextCollatz(unsigned long long n) { return n%2==0 ? n/2 : n*3+1; }
unsigned int collatzLen(unsigned long long n)
{
    unsigned int len = 1;
    while ( n > 1 )
    {
        n = nextCollatz(n);
        ++len;
    }
    return len;
}

int main(int argc, char** argv)
{
    if ( argc < 2 )
        return -1;
    unsigned n = atoi(argv[1]);

    unsigned long long sum = 0;
    int count = 0;
    for ( unsigned int i=1; ; ++i )
    {
        unsigned int len = collatzLen(i);
        if ( len >= n )
            break;
        // Main part of computing the root mean square
        sum += len*len;
        ++count;
    }
    double res = sqrt(double(sum) / double(count));
    cout << res << endl;

    return 0;
}