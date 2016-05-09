/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"


static int  my_class(size_t size);
static void *find_class(int class);
static void init_class(void);
static void in_class(void *bp);
static void out_class(void *bp);
static int  is_in_class(void *bp);
static void traverse_class(void);
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
int mm_check(void);
/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "두기의 팀",
    /* First member's full name */
    "민두기",
    /* First member's email address */
    "bellatoris@snu.ac.kr",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* Basic constants and macros */
#define WSIZE	    4	    /* Word and header/footer size (bytes) */
#define DSIZE	    8	    /* Double word size (bytes) */
#define CHUNKSIZE   (1<<12) /* Extend heap by this amout (bytes) */

#define MAX(x, y) ((x) > (y)? (x): (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)	    (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)	(GET(p) & ~0x7)
#define GET_ALLOC(p)	(GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)	((char *)(bp) - WSIZE)
#define FTRP(bp)	((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)	((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)	((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define NEXT_CLASS(bp)	((char *)(*(size_t *)(bp)))

#define CLASS 16

static char *heap_listp;
static size_t class1 = 0;
static size_t class2 = 0;
static size_t class3 = 0;
static size_t class4 = 0;
static size_t class5 = 0;
static size_t class6 = 0;
static size_t class7 = 0;
static size_t class8 = 0;
static size_t class9 = 0;
static size_t class10 = 0;
static size_t class11 = 0;
static size_t class12 = 0;
static size_t class13 = 0;
static size_t class14 = 0;
static size_t class15 = 0;
static size_t class16 = 0;



/* 
 * mm_init - initialize the malloc package.
 * such as allocating the initial heap area.
 * The return value should be -1 if there was
 * problem in performiing the initialization, 
 * 0 otherwise.
 */
int mm_init(void)
{
    init_class();
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
	return -1;
    PUT(heap_listp, 0);	    /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));	    /* Epilogue header */
    heap_listp += (2*WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
	return -1;
    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *	Always allocate a block whose size is a multiple of the alignment.
 *	The mm_mmaloc routine returns a pointer to an allocated block payload
 *	of at least size bytes. The entire allocated block should lie within
 *	the heap region and should not overlap with any other allocated chunk
 */
void *mm_malloc(size_t size)
{
    size_t asize;	    /* Adjusted block size */
    size_t extendsize;	    /* Amount to extend heap if no fit */
    char *bp;

    /* Ignore spurious requests */
    if (size == 0)
	return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
	asize = 2*DSIZE;
    else
	asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

//    printf("malloc size: %d\n", asize);
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
	place(bp, asize);
//        printf("heapsize: %d\n", mem_heapsize());
	return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    
    if (asize > CHUNKSIZE && !GET_ALLOC(((char *)mem_heap_hi() - DSIZE + 1))) {
	size_t temp = GET_SIZE(((char *)mem_heap_hi() - DSIZE + 1));
	bp = extend_heap((asize - temp)/WSIZE);
    } else {
	bp = extend_heap(extendsize/WSIZE);
    }

    if (!bp)
	return NULL;

    place(bp, asize);
//    printf("heapsize: %d\n", mem_heapsize());

    return bp;
}

/*
 * mm_free - The mm_free routine frees the block pointed to by ptr.
 *	It returns nothing. This routine is only guaranteed to work 
 *	when the passed pointer (ptr) was returned by an earlier call
 *	to mm_malloc or mm_realloc and has not yet been freed.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
//    printf("free size: %d\n", size);
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}


/*
 * mm_realloc - The mm_realloc routine returns a pointer to an allocated
 *	region of at least size bytes with the following constraints
 *
 *	    - if ptr is NULL, the call is equivalent to mm_malloc(size);
 *	    - if size is equal to zero, the call is equivalent to mm_free(ptr);
 *	    - if ptr is not NULL, it must have been returned by an earlier call
 *	      to  mm_malloc or mm_realloc. The call to mm_realloc changes the 
 *	      size of the memory block pointed to by ptr (the old block) to size
 *	      bytes and retuns the address of the new block.
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t oldsize;
    size_t asize;
    size_t temp;
    
    oldsize = GET_SIZE(HDRP(oldptr));
  
    if (size == 0) {
	mm_free(oldptr);
	return NULL;
    } else if (size <= DSIZE) {
	asize = 2*DSIZE;
    } else {
	asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

//    printf("asize: %d\n", asize);
   // traverse_class();
    if (asize < oldsize) {
	//줄이는 경우
	newptr = oldptr;
	if (oldsize - asize >= 2 * DSIZE) {
	    PUT(HDRP(newptr), PACK(asize, 1));
	    PUT(FTRP(newptr), PACK(asize, 1));
	    void *bp = NEXT_BLKP(newptr);
	    PUT(HDRP(bp), PACK(oldsize - asize, 0));
	    PUT(FTRP(bp), PACK(oldsize - asize, 0));
	    coalesce(bp);
	}
    } else if (asize > oldsize) {
	//늘리는 경우
	temp = *(size_t *)oldptr;
	if (!GET_ALLOC(HDRP(PREV_BLKP(oldptr)))) {
	    if (!GET_SIZE(HDRP(NEXT_BLKP(oldptr)))) {
		char *bp =  extend_heap((asize - oldsize)/WSIZE);
		if (!bp)
		    return NULL;
		out_class(bp);
		PUT(HDRP(oldptr), PACK(asize, 1));
		PUT(FTRP(oldptr), PACK(asize, 1));
		newptr = oldptr;
	    } else {
		if (!(newptr = mm_malloc(size)))
		    return NULL;
		memcpy(newptr, oldptr, oldsize - DSIZE);
		mm_free(oldptr);
	    }
	} else {
	    mm_free(oldptr);
	    if (!(newptr = mm_malloc(size)))
		return NULL;
	    else if(newptr != oldptr) 
		memcpy(newptr, oldptr, oldsize - DSIZE);
	    *(size_t *)newptr = temp;
	}
    } else {
	newptr = oldptr;
    }
    return newptr;
}

int mm_check()
{
    return 0;
}


/*
 *
 *
 *
 *
 *
 *
 */

static int my_class(size_t size)
{
    int i = 0;
    double dsize = size / WSIZE;
    while (dsize > 1) {
	i++;
	dsize = dsize / 2;
    }
    return i;
}

static void *find_class(int class) 
{
    switch (class) {
    case 1:
	    return &class1;
    case 2:
	    return &class2;
    case 3:
	    return &class3;
    case 4:
	    return &class4;
    case 5:
	    return &class5;
    case 6:
	    return &class6;
    case 7:
	    return &class7;
    case 8:
	    return &class8;
    case 9:
	    return &class9;
    case 10:
	    return &class10;
    case 11:
	    return &class11;
    case 12:
	    return &class12;
    case 13:
	    return &class13;
    case 14:
	    return &class14;
    case 15:
	    return &class15;
    default:
	    return &class16;
    }
}

static void init_class()
{
    int i;
    for (i = 1; i <= CLASS; i++) {
	char *class = find_class(i);
	*(size_t *)class = 0;
    }
}

static void in_class(void *bp) 
{
    size_t size = GET_SIZE(HDRP(bp));
    char *class = find_class(my_class(size));

    *(size_t *)bp = *(size_t *)class;
    *(size_t *)class = (size_t)bp;
}

static void out_class(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    char *class = find_class(my_class(size));
    char *curr;

    for (curr = class; curr != NULL; curr = NEXT_CLASS(curr)) {
	if (NEXT_CLASS(curr) == bp) {
	    *(size_t *)curr = *(size_t *)NEXT_CLASS(curr);
	    return;
	}
    }
}

static int is_in_class(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    char *class = find_class(my_class(size));
    char *curr;
    
    for (curr = class; curr != NULL; curr = NEXT_CLASS(curr)) {
	if (NEXT_CLASS(curr) == bp) {
	    return 1;
	}
    }
    return 0;
}

static void traverse_class()
{
    char *curr;
    int i;

    for (i = 1; i <= CLASS; i++) {
	char *class = find_class(i);
	printf("class%d: ", i);
	for (curr = class; curr != NULL; curr = NEXT_CLASS(curr)) {
	    printf("%p  ", NEXT_CLASS(curr));
	    if (*(size_t *)curr)
		printf("size %d  ", GET_SIZE(HDRP(NEXT_CLASS(curr))));
	}
	printf("\n");
    }
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
	return NULL;
    
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));	    /* Free blcok header */
    PUT(FTRP(bp), PACK(size, 0));	    /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   /* New epilogue header */

    /* Coalesce if the previous block or next block was free */
    return coalesce(bp);
}


static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {		/* Case 1 */
	;
    } else if (prev_alloc && !next_alloc) {	/* Case 2 */
	out_class(NEXT_BLKP(bp));
	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
    } else if (!prev_alloc && next_alloc) {	/* Case 3 */
	out_class(PREV_BLKP(bp));
	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    } else {					/* Case 4 */
	out_class(NEXT_BLKP(bp));
	out_class(PREV_BLKP(bp));
	size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
		GET_SIZE(FTRP(NEXT_BLKP(bp)));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    }
    in_class(bp);
    //traverse_class();
    return bp;
}

static void *find_fit(size_t asize)
{
    /* First fit search */
    void *bp;
    int flag = 0;
    int cls = my_class(asize);
    char *class = find_class(cls);
    
    while (cls <= CLASS || flag == 0) {
	for (bp = class; bp != NULL; bp = NEXT_CLASS(bp)) {
		if (*(size_t *)bp && asize <= GET_SIZE(HDRP(NEXT_CLASS(bp)))) {
		    return NEXT_CLASS(bp);
		}
	}
	flag = 1;
	cls++;
	class = find_class(cls);
    }
    return NULL; /* No fit */
}


/* in_class out_class통과 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    out_class(bp);
    if ((csize - asize) >= (2*DSIZE)) {
	PUT(HDRP(bp), PACK(asize, 1));
	PUT(FTRP(bp), PACK(asize, 1));
	bp = NEXT_BLKP(bp);
	PUT(HDRP(bp), PACK(csize - asize, 0));
	PUT(FTRP(bp), PACK(csize - asize, 0));
	in_class(bp);
    } else {
	PUT(HDRP(bp), PACK(csize, 1));
	PUT(FTRP(bp), PACK(csize, 1));
    }
}
