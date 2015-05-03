
#include <stdio.h>

unsigned long testAccumulate(unsigned n)
{
    unsigned long res = 0;
    for ( unsigned i=0; i<n; i++ )
    {
        for ( unsigned j=0; j<n; j++ )
        {
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
    printf("%lu\n", res);

    return 0;
}