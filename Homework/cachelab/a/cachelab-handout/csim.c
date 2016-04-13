/*
 * Name: 민두기
 * Student ID: 2012-11598
 * LRU의 경우 pseudo LRU로 구현하려 시도했었고
 * 상당히 잘 동작했지만 한가지 경우에 옳지(ref와 같지 않은)
 * 경우가 나와 timestamp를 이용하여 구현하였다.
 * ref보다 hit이 더 높은경우도 있었는데 꼭 같아야만 점수가 
 * 채점되는 방식이 조금 아쉬웠다.
 * timestamp의 경우 access, alloc, find_victim 함수에서 모두
 * 새로히 로드하거나 access한 경우 line_no - 1만큼으로 set하고
 * 그것을 제외한 다른 line들은 timestamp를 1씩 감소시켰다.
 * 물론 같은게 반복적으로 access될 수 있으니 0부터 line_no - 1까지
 * 잘 ordering 하기 위해 access하는 line보다 timestamp가 큰 
 * line들의 timestamp만 감소시켰다.
 *
 */

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
//    int LRU;
    int timestamp;
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

    int verbosity;
} Cache;

Cache* create_cache(int setsize, int ways, int blocksize, int verbosity)
{
    //allocate cache and initialize them
    int num_of_setsize = (int)pow((double)2, (double)setsize);

    Cache *my_cache = (Cache*)calloc(1, sizeof(Cache));

    my_cache->set = (Set*)malloc(sizeof(Set) * num_of_setsize);
    my_cache->setsize = setsize;
    my_cache->ways = ways;
    my_cache->blocksize = blocksize;
    my_cache->verbosity = verbosity;
    
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
    //update timestamp
    my_cache->s_hit++;
    
    int i;
    for (i = 0; i < my_cache->ways; i++) {
	l[i].timestamp = l[i].timestamp > l[index].timestamp ? l[i].timestamp - 1 : l[i].timestamp;
    }
    l[index].timestamp = my_cache->ways - 1;

    /*
    int i = index + my_cache->ways;
    while (i) {
	if(i % 2) {
	    l[i / 2].LRU = 0;
	} else {
	    l[i / 2].LRU = 1;
	}
	i = i / 2;
    }
    */
}

void line_alloc(Cache *my_cache, Line *l, int tag, int index)
{
    //update valid
    //update tag
    my_cache->s_miss++;

    l[index].valid = 1;
    l[index].tag = tag;
    int i;
    for (i = 0; i < my_cache->ways; i++) {
	l[i].timestamp = l[i].timestamp !=  0 ? l[i].timestamp - 1 : 0;
    }
    l[index].timestamp = my_cache->ways - 1; 
    /*
    int i = index + my_cache->ways;    
    while (i) {
	if(i % 2) {
	    l[i / 2].LRU = 0;
	} else {
	    l[i / 2].LRU = 1;
	}
	i = i / 2;
    } 
    */
}

void find_victim(Cache *my_cache, Line *l, int tag)
{
    // find victim and alloc
    my_cache->s_evict++;
    my_cache->s_miss++;

    int i;
    for (i = 0; i < my_cache->ways; i++) {
	if (!l[i].timestamp) {
	    l[i].valid = 1;
	    l[i].tag = tag;
	    l[i].timestamp = my_cache->ways - 1;
	} else {
	    l[i].timestamp--;
	}
    }
    /*
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
    int k = i - my_cache->ways;
    l[k].valid = 1;
    l[k].tag = tag;
    */
}

void cache_access(Cache *my_cache, unsigned long int address, int length)
{
    //compute tag and set_idx
    //
    int tag_size = 64 - (my_cache->setsize + my_cache->blocksize);
    unsigned long int tag = address >> (my_cache->setsize + my_cache->blocksize);
    unsigned long int set_idx = ((address >> my_cache->blocksize) <<
				    (tag_size + my_cache->blocksize)) >>
					(tag_size + my_cache->blocksize);
    
    Line *l = my_cache->set[set_idx].way;

    int verbosity = my_cache->verbosity;
    int i;
    
    for (i = 0; i < my_cache->ways; i++) {
	if (!l[i].valid) {
	    if (verbosity) printf("miss ");
	    line_alloc(my_cache, l, tag, i);
	    break;
	} else if (l[i].tag == tag) {
    	    if (verbosity) printf("hit ");
	    line_access(my_cache, l, i);
	    break;
	} else if (i == my_cache->ways - 1) {
	    if (verbosity) printf("miss eviction ");
	    find_victim(my_cache, l, tag);
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
    long int v = 0, s = 0, E = 0, b = 0;
    char t[64];
    int c;
    FILE *fp;
    Cache *cache;
    //use getopt for option
    while ((c = getopt(argc, argv, "hvs:E:b:t:"))!= -1) {
	switch(c) {
	case 'h':
	    help();
	    break;
	case 'v':
	    v = 1;
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

    cache = create_cache(s, E, b, v);
    
    fp = fopen(t, "r");

    if(!s || !E || !b || !fp) {
	return -1;
    }

    char *type = NULL;
    unsigned long int address;
    int length;

    //cache access line by line
    while (fscanf(fp, "%ms %lx,%x", &type, &address, &length) != EOF) {
	if (type[0] != 'I') printf("%s %lx,%x ", type, address, length);
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
	if(type[0] != 'I') printf("\n");
	if (type != NULL) {
	    free(type);
	    type = NULL;
	}
    }
    
    printSummary(cache->s_hit, cache->s_miss, cache->s_evict);
    delete_cache(cache);
    return 0;
}
