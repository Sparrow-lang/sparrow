
#include <iostream>

using namespace std;

int fibonacciRec(int n)
{
    if ( n == 0 )
        return 0;
    if ( n == 1 )
        return 1;

    return fibonacciRec(n - 1) + fibonacciRec(n - 2);
}

void fibonacciRec2(int n, int& res)
{
    if ( n == 0 )
        res = 0;
    else if ( n == 1 )
        res = 1;
    else
    {
        int r1, r2;
        fibonacciRec2(n-1, r1);
        fibonacciRec2(n-2, r2);
        res = r1 + r2;
    }
}


int main(int argc, const char** argv)
{
    if ( argc < 2 )
        return -1;
    int n = atoi(argv[1]);
    cout << fibonacciRec(n);
    // int res;
    // fibonacciRec2(n, res);
    // cout << res;
    return 0;
}