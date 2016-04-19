#include <stdio.h>
#include <stdlib.h>
int main()
{
    int *i = (int *)malloc(sizeof(int));
    free(i);
    return 0;
}
