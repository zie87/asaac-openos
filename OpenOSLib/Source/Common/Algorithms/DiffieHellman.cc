#include "DiffieHellman.hh"

//"Square&Multiply" algorithm
unsigned long acmodm(unsigned long a, unsigned long c, long m)
{
    unsigned long res = 1;
    
    for (unsigned long i = 1<<(sizeof(c)*8 - 1); i > 0; i >>= 1)
    {
        res = div( res*res, m ).rem;

        if ((c & i) == i)
            res = div( res*a, m ).rem;
    }
    
    return res;
}
