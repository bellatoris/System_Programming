#include <setjmp.h>
#include <stdio.h>
jmp_buf buf;

void p1();
void p2();
void p3();

int main() 
{
    if (setjmp(buf) != 0) 
	printf("back in main due to an error\n");
    else
	printf("first time through\n");
    p1();
}


void p1()
{
    p2();
}

void p2()
{
    p3();
}

void p3()
{
    longjmp(buf, 1);
}
