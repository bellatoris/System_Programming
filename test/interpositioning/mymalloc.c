#ifdef RUNTIME
#define __USE_GNU
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>


void *malloc(size_t size)
{
    static void *(*mallocp)(size_t size);
    char *error;
    void *ptr;

    /* get addresee of lib malloc */
    if (!mallocp) {
	mallocp = dlsym(RTLD_NEXT, "malloc");
	if ((error = dlerror()) != NULL) {
	    fputs(error, stderr);
	    exit(1);
	}
    }
    ptr = mallocp(size);
    printf("malloc(%d) = %p\n", (int)size, ptr);
    return ptr;
}

void free(void *ptr) 
{
    static void (*freep)(void *);
    char *error;

    if (!freep) {
	freep  = dlsym(RTLD_NEXT, "free");
	if ((error = dlerror()) != NULL) {
	    fputs(error, stderr);
	    exit(1);
	}
    }
    printf("free(%p)\n", ptr);
    freep(ptr);
}
#endif
