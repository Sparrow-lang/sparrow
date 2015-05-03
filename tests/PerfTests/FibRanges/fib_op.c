
#include <stdio.h>

unsigned int fib(unsigned int n) {
    unsigned int a = 0;
    unsigned int b = 1;
    for ( int i=0; i<n; ++i ) {
        unsigned int aOld = a;
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

    unsigned int res = 0;
    for ( int i=1; i<=n; ++i )
    {
        unsigned int f = fib(i);
        if ( f % 2 != 0 )
            res += f*f;
    }
    printf("%lu\n", res);

    return 0;
}