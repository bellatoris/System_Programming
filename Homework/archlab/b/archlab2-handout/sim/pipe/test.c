#include <stdio.h>

int src[8], dst[8];

/* $begin ncopy */
/*
 * ncopy - copy src to dst, returning number of positive ints
 * contained in src array.
 */
int ncopy(int *src, int *dst, int len)
{
    int count = 0;
    int val;

    while (len > 0) {
	val = *src++;
	*dst++ = val;
	if (val > 0)
	    count++;
	len--;

	if (len <= 0)
	    break;

	val = *src++;
	*dst++ = val;
	if (val > 0)
	    count++;
	len--;

	if (len <= 0)
	    break;
    
	val = *src++;
	*dst++ = val;
	if (val > 0)
	    count++;
	len--;
    }
    return count;
}
/* $end ncopy */

int main()
{
    int i, count;
}


