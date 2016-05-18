/*
 * Name: 민두기
 * Student ID: 2012-11598
 *
 * Segregated list로 구현하였으며 각 class는 address-order로 정렬
 * 되어 있다. n-th class는 2^n 부터 2^(n+1)-1 word를 가지고 있는 
 * block을 담을 수 있다. 각각의 class들의 header를 위해 15개의size_t를
 * global하게 정의하였다. (array를 사용할 수 없게 되어있어서 하나 하나 
 * 정의했다, 그냥 array를 사용할 수 있게 해주는게 훨씬 깔끔한 구현이 
 * 되었을텐데 이러한 제약을 둔것이 아쉬웠다.) word_size-order로 list를 
 * 정렬했다면 first-fit을 best-fit으로 만들 수 있겠지만, address-order로
 * 구현 하는것이 reallocation시에 memory utilization을 더 좋게 할 수 
 * 있었다. memory allocate시 byte를 받아와 어느 class에 들어가야 할지 
 * 계산한 후 그 class를 traverse하면서 size보다 큰 첫번째 free block을
 * (first-fit)을 찾아 할당해준다. 현재 class에 없다면 다음 class로 가고
 * 모든 class에서 맞는 크기를 찾을 수 없다면 heap size를 extend하여
 * 할당해준다.
 *
 * 각각의 block은 4-byte header와 4-byte footer를 가지고 있다. 기본적인 
 * 구현이나 매크로는 교과서를 참고하였다. header와 footer에는 현재 block의
 * 전체 size와 그 block이 allocation되어 있는지 free block인지의 정보를
 * 담아 둔다.
 *
 * 함수에 대한 설명은 함수 위쪽에 적어 두었습니다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"


static int	my_class(size_t size);
static void	*find_class(int class);
static void	init_class(void);
static void	in_class(void *bp);
static void	out_class(void *bp);
static void	is_in_class(void *bp);
static void	traverse_class(void);
static size_t	get_asize(size_t size);
static void	*extend_heap(size_t words);
static void	*coalesce(void *bp);
static void	*find_fit(size_t asize);
static void	*place(void *bp, size_t asize);

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

/* Given block ptr bp, compute address of next free blocks */
#define NEXT_CLASS(bp)	((char *)(*(size_t *)(bp)))

#define CLASS 16	/* Number of class */

static char *heap_listp;
//static size_t class1 = 0;
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
 * The return value is -1 if there was
 * problem in performiing the initialization, 
 * 0 otherwise.
 */
int mm_init(void)
{
    /* Initialize list header */
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
    asize = get_asize(size);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
	bp = place(bp, asize);
	return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    /* 만약 heap의 마지막부분이 free block이라면 필요한 size - free block의 size
     * 만큼만 heap size를 키운다. 이렇게 함으로써 불필요하게 heap size를 늘리는 
     * 것을 막을 수 있다.
     */
    if (asize > CHUNKSIZE && !GET_ALLOC(((char *)mem_heap_hi() - DSIZE + 1))) {
	size_t temp = GET_SIZE(((char *)mem_heap_hi() - DSIZE + 1));
	bp = extend_heap((asize - temp)/WSIZE);
    } else {
	bp = extend_heap(extendsize/WSIZE);
    }

    if (!bp)
	return NULL;
    bp = place(bp, asize);

    return bp;
}

/*
 * mm_free - The mm_free routine frees the block pointed to by ptr.
 *	It returns nothing. This routine is only guaranteed to work 
 *	when the passed pointer (ptr) was returned by an earlier call
 *	to mm_malloc or mm_realloc and has not yet been freed.
 *	block의 header와 footer에 free되었다고 표시를 한후 
 *	coalescing을 한다.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
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
 *	    
 * realloc의 경우 불필요한 data의 복사나 heap size의 증가를 막기 위해서
 * case를 나눠서 진행하였다. 우선 크기를 줄이는 경우에는 pointer를 이동하거나
 * 복사하지 않고 그대로 현재 block size를 줄이기만 했다.
 * 크기를 늘리는 경우에는 현재 block의 prev block이 free block인지 check 한다.
 * free block이 아니라면 현재 block을 free한후 다시 늘어난 size 만큼 malloc을
 * 한다. 이때 현재 block의 next block이 free block이거나, 마지막 block이었다면,
 * size를 늘리더라도 pointer의 이동은 일어나지 않는다. 그러므로 그대로 return.
 * 만약 pointer의 이동이 일어났다면, oldptr의 data를 newptr에 복사해준다.
 * 현재 block의 prev block이 free block이라면 현재 block을 free하는 순간
 * data를 복사해줘야 하고 memory utilization이 떨어지게 된다. 이 때는 현재
 * block이 마지막 block이라면, extend heap을 해주고, 그렇지 않다면 그냥
 * 새로히 mm_malloc을 불러 reallocation 해준다.
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t oldsize;
    size_t asize;
  
    if (size == 0) {
	mm_free(oldptr);
	return NULL;
    } else if (!ptr) {
	oldptr = mm_malloc(size);
	return oldptr;
    }

    oldsize = GET_SIZE(HDRP(oldptr));
    asize = get_asize(size);

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
	size_t temp = *(size_t *)oldptr;
	if (!GET_ALLOC(HDRP(PREV_BLKP(oldptr)))) {
	    if (!GET_SIZE(HDRP(NEXT_BLKP(oldptr)))) {
		void *bp =  extend_heap((asize - oldsize)/WSIZE);
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
    char *curr = heap_listp;
    while (GET_SIZE(NEXT_BLKP(curr))) {
	if (!GET_ALLOC(HDRP(curr))) {
	    is_in_class(curr);	    
	}
	
	if ((size_t)HDRP(curr) + GET_SIZE(HDRP(curr)) > 
			    (size_t)HDRP(NEXT_BLKP(curr))) {
	    printf("this block overlays next block\n");
	}
	curr = NEXT_BLKP(curr);
    }
    traverse_class();
    return 0;
}



/*
 * help function들이다.
 */

/*
 * my_class - size에 맞는 class number를 return 해준다.
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

/*
 * find_class - class에 맞는 header를 return 해준다.
 */
static void *find_class(int class) 
{
    switch (class) {
    case 1:
	    return NULL;
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

/*
 * init_class - header안에 담겨있는 pointer들을 모두 0으로 만들어준다.
 */
static void init_class()
{
    int i;
    for (i = 2; i <= CLASS; i++) {
	char *class = find_class(i);
	*(size_t *)class = 0;
    }
}

/*
 * in_class - block ptr을 받아서 알맞은 class에 address order로 넣어준다.
 */
static void in_class(void *bp) 
{
    size_t size = GET_SIZE(HDRP(bp));
    char *class = find_class(my_class(size));
    char *curr;

    for (curr = class; curr != NULL; curr = NEXT_CLASS(curr)) {
//	if (*(size_t *)curr == 0 || size < GET_SIZE(HDRP(NEXT_CLASS(curr)))) {
	if (*(size_t *)curr == 0 || (size_t)bp < (size_t)NEXT_CLASS(curr)) {
	    *(size_t *)bp = *(size_t *)curr;
	    *(size_t *)curr = (size_t)bp;
	    break;
	}
    }
}

/*
 * out_class - block ptr을 받아서 class에서 제거한다.
 */
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

static void is_in_class(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    char *class = find_class(my_class(size));
    char *curr;

    for (curr = class; curr != NULL; curr = NEXT_CLASS(curr)) {
	if (NEXT_CLASS(curr) == bp) {
	    return;
	}
    }
    printf("this block is not in the class list\n");
}

/*
 * traverse_class - 모든 class를 순회한다.
 */
static void traverse_class()
{
    char *curr;
    int i;

    for (i = 2; i <= CLASS; i++) {
	char *class = find_class(i);
	for (curr = class; curr != NULL; curr = NEXT_CLASS(curr)) {
	    if (NEXT_CLASS(curr)) {
		if (GET_ALLOC(HDRP(NEXT_CLASS(curr)))) {
		    printf("this block is not free\n");
		}

		if (!GET_ALLOC(HDRP(NEXT_BLKP(NEXT_CLASS(curr)))) ||
			!GET_ALLOC(HDRP(PREV_BLKP(NEXT_CLASS(curr))))) {
		    printf("coalescing is not successful\n");
		}

		if ((size_t)NEXT_CLASS(curr) > (size_t)mem_heap_hi() || 
			(size_t)NEXT_CLASS(curr) < (size_t)mem_heap_lo()) {
		    printf("this block ptr is not valid\n");
		}

		if (i != my_class(GET_SIZE(HDRP(NEXT_CLASS(curr))))) {
		    printf("this block is in wrong class\n");
		}
	    }
	    //printf("%p  ", NEXT_CLASS(curr));
	    //if (*(size_t *)curr)
	//	printf("size %d  ", GET_SIZE(HDRP(NEXT_CLASS(curr))));
	}
    }
}

/*
 * get_asize - size를 받아서 align된 asize를 return 해준다. 
 */ 
static size_t get_asize(size_t size) 
{
    size_t asize;

    if (size <= DSIZE) {
	asize = 2*DSIZE;
    } else {
	asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }
    
    return asize;
}

/*
 * extend_heap - words를 받아와서 그 수 만큼 heap크기를 늘린다.
 */
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

/*
 * coalesce - block ptr을 받아와서 bp 앞뒤로 free block이 있으면
 *	     합쳐준다. case를 4개로 나눠서 구현하였다.
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {		/* Case 1 앞뒤 모두 free block이 아닐경우 */
	;
    } else if (prev_alloc && !next_alloc) {	/* Case 2 앞의 block만 free block일 경우 */
	out_class(NEXT_BLKP(bp));
	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
    } else if (!prev_alloc && next_alloc) {	/* Case 3 뒤의 block만 free block일 경우*/
	out_class(PREV_BLKP(bp));
	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    } else {					/* Case 4 앞뒤 모두 free block일 경우*/
	out_class(NEXT_BLKP(bp));
	out_class(PREV_BLKP(bp));
	size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
		GET_SIZE(FTRP(NEXT_BLKP(bp)));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    }
    in_class(bp);
    return bp;
}

/*
 * find_fit - asize를 받아서 알맞은 class list를 순회 하면서 
 *	     asize보다 큰 size를 가진 block을 first fit으로 찾아
 *	     return 한다. 찾을 수 없다면 NULL을 return한다.
 */
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


/* 
 * place - free block ptr과 asize를 받아서 allocated block으로 header를 바꿔준다.
 *	    free block의 size가 asize보다 2 * DSIZE 보다 크다면, 남은 size만큼의
 *	    free block을 만들어준다.
 *	    asize < 100이라면 free block의 뒤쪽에다가 allocation한다. 이렇게 
 *	    함으로써 reallocation시 utilization을 높일 수 있다. 
 */
static void *place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    size_t rsize = csize - asize;
    out_class(bp);

    if (rsize >= 2 * DSIZE) {
	if (asize < 100) {
	    PUT(HDRP(bp), PACK(rsize, 0));
	    PUT(FTRP(bp), PACK(rsize, 0));
	    PUT(HDRP(NEXT_BLKP(bp)), PACK(asize, 1));
	    PUT(FTRP(NEXT_BLKP(bp)), PACK(asize, 1));
	    in_class(bp);
	    bp = NEXT_BLKP(bp);
	} else  {
	    PUT(HDRP(bp), PACK(asize, 1));
	    PUT(FTRP(bp), PACK(asize, 1));
	    bp = NEXT_BLKP(bp);
	    PUT(HDRP(bp), PACK(csize - asize, 0));
	    PUT(FTRP(bp), PACK(csize - asize, 0));
	    in_class(bp);
	    bp = PREV_BLKP(bp);
	}
    } else {
	PUT(HDRP(bp), PACK(csize, 1));
	PUT(FTRP(bp), PACK(csize, 1));
    }
    mm_check();
    return bp;
}
