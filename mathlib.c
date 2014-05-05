#include "mathlib.h"

// x^y power.
/*
    IN:    x, y
    OUT:   x^y
    WHAT:  integer exponent
*/
int
power(int x, int y)
{
    int i=0, result=1;
    
    if(y==0)
        return 1;
    if(y<0)
    {
        printf("Error in operand y. Returning 0.\n");
        return 0;
    }
    
    for(; i<y; i++)
    {
        result *= x;
    }
    
    return result;
}

/*
    IN:    x
    OUT:   |x|
    WHAT:  absolute value
*/
int
abs(int x)
{
    if(x<0)
        return -x;
    else
        return x;
}

/*
    IN:    array-apply operation to this
            n-number of entries to write
            x-the number to set to
    OUT:   NO
    WHAT:  set array up to index n to x
*/
void
setNInt(int32_t * array, int32_t n, int32_t x)
{
    int i=0;
    
    if(array == NULL)
        return;
    
    for(i=0; i<n; i++)
    {
        array[i]=x;
    }
}

/*
    IN:    array
            a-index begin
            b-index end
            x-set to value
    OUT:   NO
    WHAT:  set all from [a,b] to x
*/
void
blkSetInt(int32_t * array, int32_t a, int32_t b, int32_t x)
{
    int i=0;
    
    if(array == NULL)
        return;
    
    for(i=a; i<b; i++)
    {
        array[i]=x;
    }
}
