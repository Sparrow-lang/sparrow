
#include <iostream>

using namespace std;

unsigned long fib(unsigned long n) {
    unsigned long a = 0;
    unsigned long b = 1;
    for ( int i=0; i<n; ++i ) {
        unsigned long aOld = a;
        a = b;
        b += aOld;
    }
    return b;
}

int main(int argc, char** argv)
{
    if ( argc < 2 )
        return -1;
    unsigned n = atoi(argv[1]);

    unsigned long res = 0;
    for ( int i=1; i<=n; ++i )
    {
        unsigned long f = fib(i);
        if ( f % 2 != 0 )
            res += f*f;
    }
    cout << res << endl;

    return 0;
}