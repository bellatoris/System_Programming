#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void s(int d) {
      printf("begin (%d)\n", d);
        sleep(5);
	  printf("end   (%d)\n", d);
}

int main() {
      int pid = getpid();
        signal(SIGINT, s);
	  signal(SIGCHLD, s);
	    if (fork() == 0) {
		    kill(pid, SIGINT);
		        sleep(1);
			    kill(pid, SIGINT);
			        sleep(1);
				    exit(0);
				      }
	      for(;;);
}
