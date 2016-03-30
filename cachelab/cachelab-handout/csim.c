#include "cachelab.h"
#include <assert.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct __line {
    unsigned long int tag;
    int valid;
    int LRU;
} Line;

typedef struct __set {
    Line *way;	    //cache lines
} Set;

typedef struct __cache {
    Set *set;	    //cache sets
    
    int setsize;
    int ways;
    int blocksize;

    int s_hit;
    int s_miss;
    int s_evict;
} Cache;

Cache* create_cache(int setsize, int ways, int blocksize)
{
    //allocate cache and initialize them
    //power of 2
    int num_of_setsize = (int)pow((double)2, (double)setsize);

    Cache *my_cache = (Cache*)calloc(1, sizeof(Cache));

    my_cache->set = (Set*)malloc(sizeof(Set) * num_of_setsize);
    my_cache->setsize = setsize;
    my_cache->ways = ways;
    my_cache->blocksize = blocksize;
    
    int i;
    
    for(i = 0; i < num_of_setsize; i++)
	my_cache->set[i].way = (Line*)calloc(ways, sizeof(Line));
    
    return my_cache;
}

void delete_cache(Cache *my_cache)
{
    int i;
    for(i = 0; i < (int)pow((double)2, (double)my_cache->setsize); i++)
	free(my_cache->set[i].way);
    free(my_cache->set);
    free(my_cache);
}

void line_access(Cache *my_cache, Line *l, int index)
{
    //update LRU bit of Line
    my_cache->s_hit++;

    int i = index + my_cache->ways;

    while (i) {
	if(i % 2) {
	    l[i / 2].LRU = 0;
	} else {
	    l[i / 2].LRU = 1;
	}
	i = i / 2;
    }
}

void line_alloc(Cache *my_cache, Line *l, int tag, int index)
{
    //flip the LRU bit
    my_cache->s_miss++;

    l[index].valid = 1;
    l[index].tag = tag;

    int i = 1;

    while (i < my_cache->ways) {
	if (l[i].LRU) {
	    l[i].LRU = 0;
	    i = 2 * i + 1;
	} else {
	    l[i].LRU = 1;
	    i = 2 * i;
	}
    }
   //update data structrues to reflect allocation of a new block into a line
}

int set_find_victim(Cache *my_cache, Line *l)
{
    // for a given set, return the victim line where to place the new block
    my_cache->s_evict++;

    int i = 1;

    while (i < my_cache->ways) {
	if (l[i].LRU) {
	    i = 2 * i + 1;
	} else {
	    i = 2 * i;
	}
    }
    return i - my_cache->ways;
}

void cache_access(Cache *my_cache, unsigned long int address, int length)
{ 
    int tag_size = 64 - (my_cache->setsize + my_cache->blocksize);
    unsigned long int tag = address >> (my_cache->setsize + my_cache->blocksize);
    unsigned long int set_idx = ((address >> my_cache->blocksize) << (tag_size + my_cache->blocksize)) >> (tag_size + my_cache->blocksize);
    //printf("%lx, %lx, %d\n", tag, set_idx, tag_size);
    Line *l = my_cache->set[set_idx].way;

    int i;
    
    for (i = 0; i < my_cache->ways; i++) {
	if (!l[i].valid) {
	    printf("miss ");
	    line_alloc(my_cache, l, tag, i);
	    break;
	} else if(l[i].tag == tag) {
    	    printf("hit ");
	    line_access(my_cache, l, i);
	    break;
	} else if (i == my_cache->ways - 1) {
	    printf("miss eviction ");
	    int k = set_find_victim(my_cache, l);
	    line_alloc(my_cache, l, tag, k);
	    break;
	}
    } 
} 

void help()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
    "Options:\n"
    "-h         Print this help message.\n"
    "-v         Optional verbose flag.\n"
    "-s <num>   Number of set index bits.\n"
    "-E <num>   Number of lines per set.\n"
    "-b <num>   Number of block offset bits.\n"
    "-t <file>  Trace file.\n"
    "\n"
    "Examples:\n"
    "linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n"
    "linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

int main(int argc, char *argv[])
{
    extern char *optarg;
    long int v, s, E, b;
    char t[64];
    int c;
    FILE *fp;
    Cache *cache;

    while ((c = getopt(argc, argv, "hvs:E:b:t:"))!= -1) {
	switch(c) {
	case 'h':
	    help();
	    break;
	case 'v':
	    v++;
	    break;
	case 's':
	    s = strtol(optarg, NULL, 0);
	    break;
	case 'E':
	    E = strtol(optarg, NULL, 0);
	    break;
	case 'b':
	    b = strtol(optarg, NULL, 0);
	    break;
	case 't':
	    strcpy(t, optarg);
	    break;
	}
    }

    cache = create_cache(s, E, b);
    
    fp = fopen(t, "r");
    char *type = NULL;
    unsigned long int address;
    int length;

    char *line = NULL;
    size_t ll = 0;
    ssize_t ilen = getline(&line, &ll, fp);

    fflush(stdout);
    if (v)
	printf("\n");

    while (ilen > 0) {
	if (sscanf(line, "%ms %lx,%x", &type, &address, &length) == 3) {
	    printf("%s %lx,%x ", type, address, (unsigned int)length);
	    switch (type[0]) {
	    case 'I':
		break;
	    case 'M':
	    case 'L':
		cache_access(cache, address, length);
		if (type[0] == 'L') break;
	    case 'S':
		cache_access(cache, address, length);
		break;
	    }
	}
	if (type != NULL) {
	    free(type);
	    type = NULL;
	}
	ilen = getline(&line, &ll, fp);
	printf("\n");
    }
    
    printSummary(cache->s_hit, cache->s_miss, cache->s_evict);
    delete_cache(cache);
    return 0;
}
