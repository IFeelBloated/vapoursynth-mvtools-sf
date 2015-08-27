#ifndef __COMMON_F__
#define __COMMON_F__

// returns a > 0 ? a : 0
inline static int satz(int a)
{
    return ~(a >> (sizeof(int)*8 - 1)) & a;
}

// returns maximum(a, b)
inline static int imax(int a, int b)
{
    return a + satz(b - a);
}

// returns minimum(a, b)
inline static int imin(int a, int b)
{
    return a - satz(a - b);
}

/* returns the biggest integer x such as 2^x <= i */
inline static int ilog2(int i)
{
    int result = 0;
    while ( i > 1 ) { i /= 2; result++; }
    return result;
}

/* computes 2^i */
inline static int iexp2(int i)
{
    return 1 << satz(i);
    //     int result = 1;
    //     while ( i > 0 ) { result *= 2; i--; }
    //     return result;
}

#endif
