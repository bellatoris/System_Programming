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

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
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

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* header size is 8 bytes */
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

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
#define NEXT(bp)	((char *)(bp))
#define PREV(bp)	((char *)(bp) + WSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)	((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)	((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

static int find_class(int size)
{
    int i = 0;
    double dsize = size / WSIZE;
    while (dsize > 1) {
	i++;
	dsize = dsize / 2;
    }
    return i;
}

static char *heap_listp;
static char *class_listp;

/* 
 * mm_init - initialize the malloc package.
 * such as allocating the initial heap area.
 * The return value should be -1 if there was
 * problem in performiing the initialization, 
 * 0 otherwise.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
	return -1;
    PUT(heap_listp, 0);	    /* Alignment padding */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /* Prologue header */
    PUT(heap_listp + (3*WSIZE), PACK(DSIZE, 1));    /* Prologue footer */
    PUT(heap_listp + (4*WSIZE), PACK(0, 1));	    /* Epilogue header */
    class_listp = heap_listp + (1*WSIZE);
    heap_listp += (3*WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
	return -1;
    return 0;
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

    /* Coalesce if the previous block was free */
    return coalesce(bp);
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

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
	place(bp, asize);
	return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
	return NULL;
    place(bp, asize);
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

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {		/* Case 1 */
	return bp;
    } else if (prev_alloc && !next_alloc) {	/* Case 2 */
	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
    } else if (!prev_alloc && next_alloc) {	/* Case 3 */
	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    } else {					/* Case 4 */
	size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
		GET_SIZE(FTRP(NEXT_BLKP(bp)));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    }

    int class = find_class(GET_SIZE(HDRP(bp)));

    return bp;
}

static void *find_fit(size_t asize)
{
    /* First fit search */
    void *bp;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
	if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
	    return bp;
	}
    }
    return NULL; /* No fit */
}

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2*DSIZE)) {
	PUT(HDRP(bp), PACK(asize, 1));
	PUT(FTRP(bp), PACK(asize, 1));
	bp = NEXT_BLKP(bp);
	PUT(HDRP(bp), PACK(csize - asize, 0));
	PUT(FTRP(bp), PACK(csize - asize, 0));
    } else {
	PUT(HDRP(bp), PACK(csize, 1));
	PUT(FTRP(bp), PACK(csize, 1));
    }
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
 *	      bytes and retuns the address of the new block. Notice that the 
 *	      address of the new block might be the same as the old block, or it
 *	      might be different, depending on your implementation, the amount of 
 *	      internal fragmentation in the old block, and the size of the realloc
 *	      request.
 *	      The contents of the new block are same as those of the old ptr block,
 *	      up to the minimum of the old and new sizes. Everyting else is 
 *	      uninitialized. For example, if the old block is 8 bytes and the new 
 *	      block is 12 bytes, then the first 8 bytes of the new block are identical
 *	      to the first 8 bytes of the old block and the last 4 bytes are 
 *	      unintialized. Similarly, if the old block is 8 bytes and the new block
 *	      is 4 bytes, then the contents of the new blcok are identical to the first
 *	      4 bytes of the old block.
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

int mmcheck()
{
    return 0;
}

