#include <stdio.h>

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;



int main()
{
    fprintf(stdout, "Hello, world\n");
}
