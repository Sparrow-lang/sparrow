
#include <iostream>

using namespace std;

unsigned long testAccumulate(unsigned n)
{
    unsigned long res = 0;
    unsigned i = 0;
    while ( i < n)
    {
        unsigned j = 0;
        while ( j < n)
        {
            res += i*j;
            ++j;
        }
        ++i;
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