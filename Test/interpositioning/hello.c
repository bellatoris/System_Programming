#include <stdio.h>
#include <malloc.h>
int main()
{
    int *i = (int *)malloc(sizeof(int));
    free(i);
    return 0;
}
