#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <wait.h>
#define N 5
typedef void (*myfork)(void);

void fork1(void)
{
    int x = 1;
    pid_t pid = fork();
    if (pid == 0) {
	printf("Child has x = %d\n", ++x);
    } else {
	printf("Parent has x = %d\n", --x);
    }
    printf("Bye from process %d with x = %d\n", getpid(), x);
}

void fork2(void)
{
    printf("L0\n");
    fork();
    printf("L1\n");
    fork();
    printf("BYE\n");
}
void fork3(void)
{
    printf("L0\n");
    fork();
    printf("L1\n");
    fork();
    printf("BYE\n");
}
void fork4(void)
{
    printf("L0\n");
    fork();
    printf("L1\n");
    fork();
    printf("BYE\n");
}
void fork5(void)
{
    printf("L0\n");
    fork();
    printf("L1\n");
    fork();
    printf("BYE\n");
}
void cleanup(void)
{
    printf("cleaning up\n");
}

void fork6(void)
{
    atexit(cleanup);
    fork();
    exit(0);
}

void fork7(void)
{
    if (fork() == 0) {
	printf("Terminationg Child, PID = %d\n", getpid());
	exit(0);
    } else {
	printf("Running Parent, PID = %d\n", getpid());
	while(1);
    }
}

void fork8(void)
{
    if (fork() == 0) {
	printf("Running Child, PID = %d\n", getpid());
	while(1);
    } else {
	printf("Running Parent, PID = %d\n", getpid());
	exit(0);
    }
}


void fork9(void)
{
    int child_status;

    if (fork() == 0) {
	printf("HC: hello from child\n");
    } else {
	printf("HP: hello from parent\n");
	wait(&child_status);
	printf("CT: child has terminated\n");
    }
    printf("Bye\n");
    exit(0);
}
void fork10(void)
{
    pid_t pid[N];
    int i;
    int child_status;
    for (i = 0; i < N; i++)
	if ((pid[i] = fork()) == 0)
	    exit(100 + i); //child die
    for (i = 0; i < N; i++) {
	pid_t wpid = wait(&child_status);
	if (WIFEXITED(child_status)) {
	    //printf("%d\n", child_status);
	    printf("Child %d terminated with exit status %d\n", wpid, WEXITSTATUS(child_status));
	} else
	    printf("Child %d terminate abnormally\n", wpid);
    }
    while(1);

}

void fork11(void)
{
    pid_t pid;
    if ((pid = fork()) == 0) {
	execve("hello", NULL, NULL);
    }
}
void fork12(void)
{
    pid_t pid[N];
    int i;
    int child_status;
    for (i = 0; i < N; i++)
	if ((pid[i] = fork()) == 0)
	    while (1);
    for (i = 0; i< N; i++) {
	printf("Killing process %d\n", pid[i]);
	kill(pid[i], SIGINT);
    }

    for (i = 0; i < N; i++) {
	pid_t wpid = wait(&child_status);
	if (WIFEXITED(child_status)) {
	    //printf("%d\n", child_status);
	    printf("Child %d terminated with exit status %d\n", wpid, WEXITSTATUS(child_status));
	} else
	    printf("Child %d terminate abnormally\n", wpid);
    }
}

void int_handler(int sig) 
{
    printf("Process %d received signal %d\n", getpid(), sig);
    exit(0);
}
void fork13(void)
{
    pid_t pid[N];
    int i;
    int child_status;
    signal(SIGINT, int_handler);
    for (i = 0; i < N; i++)
	if ((pid[i] = fork()) == 0)
	    while (1);
    for (i = 0; i< N; i++) {
	printf("Killing process %d\n", pid[i]);
	kill(pid[i], SIGINT);
    }

    for (i = 0; i < N; i++) {
	pid_t wpid = wait(&child_status);
	if (WIFEXITED(child_status)) {
	    //printf("%d\n", child_status);
	    printf("Child %d terminated with exit status %d\n", wpid, WEXITSTATUS(child_status));
	} else
	    printf("Child %d terminate abnormally\n", wpid);
    }
}

int count = 0;
void child_handler(int sig) 
{
    int child_status;
    pid_t pid = wait(&child_status);
    count--;
    printf("Received signal %d from process %d\n", sig, pid);
}

void child_handler2(int sig)
{
    int child_status;
    pid_t pid;
    while ((pid = waitpid(-1, &child_status, WNOHANG)) > 0) {
	count--;
	printf("Received signal %d from process %d\n", sig, pid);
    }
}
void fork14(void)
{
    pid_t pid[N];
    int i;
    count = N;
    signal(SIGCHLD, child_handler);
    for (i = 0; i < N; i++)
	if ((pid[i] = fork()) == 0) {
	    sleep(1);
	    exit(0);
	}
    while (count > 0)
	pause();
}
void fork15(void)
{
    pid_t pid[N];
    int i;
    count = N;
    signal(SIGCHLD, child_handler2);
    for (i = 0; i < N; i++)
	if ((pid[i] = fork()) == 0) {
	    sleep(1);
	    exit(0);
	}
    while (count > 0)
	pause();
}
int main(int argc, char *argv[])
{
    myfork mr;
    int s = atoi(argv[1]);
    switch (s) {
    case 8:
	mr = fork8;
	break;
    case 10:
	mr = fork10;
	break;
    case 11:
	mr = fork11;
	break;
    case 12:
	mr = fork12;
	break;
    case 13:
	mr = fork13;
	break;
    case 14:
	mr = fork14;
	break;
    case 15:
	mr = fork15;
	break;
    }
    mr();

}
