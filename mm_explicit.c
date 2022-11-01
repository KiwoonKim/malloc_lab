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

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
	/* Team name */
	"2조",
	/* First member's full name */
	"김기운",
	/* First member's email address */
	"atthenoon@gmail.com",
	/* Second member's full name (leave blank if none) */
	"",
	/* Second member's email address (leave blank if none) */
	""
};

/* single word (4) or double word (8) alignment */
#define WSIZE		4
#define ALIGNMENT	8
#define CHUNKSIZE	(1<<12)
#define EXPLICIT	TRUE
# define MAX(x , y) ((x) > (y)? (x) : (y))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size)			(((size) + (ALIGNMENT-1)) & ~0x7)
#define PACK(size, alloc)	((size) | (alloc))

// read & write a word at addr p
#define GET(p)		(*(unsigned int*)(p))
#define PUT(p, val)	(*(unsigned int*)(p) = (val))

// read size and alloc fields from addr p
#define GET_SIZE(p)		(GET(p) & ~0x7)
#define GET_ALLOC(p)	(GET(p) & 0x1)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define HDRP(bp)	((char *)(bp) - WSIZE)
#define FTRP(bp)	((char *)(bp) + GET_SIZE(HDRP(bp)) - ALIGNMENT)

// for decplicit lsist
#define NEXTP(bp)	(*(char **)(bp+WSIZE))
#define PREVP(bp)	(*(char **)(bp))
// given block ptr bp, compute addr of next and previous blocks
#define NEXT_BLKP(bp)	((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)	((char *)(bp) - GET_SIZE(((char *)(bp) - ALIGNMENT)))

/* 
 * mm_init - initialize the malloc package.
 */
typedef	unsigned int size_t;
int			mm_init(void);
void		mm_free(void *bp);
static void	*coalesce(void *bp);
static void	place(void *bp, size_t size);
static void *find_fit(size_t size);
static void *extend_heap(size_t words);
static char	*heap_listq = NULL;
// for explicit list
#ifdef EXPLICIT
static char *root = NULL;
static void put_block(char *bp);
static void remove_block(char *bp);
#endif
int mm_init(void)
{
	if ((heap_listq = mem_sbrk(6*WSIZE)) == NULL) return -1;
	PUT(heap_listq , 0);
	PUT(heap_listq + (1*WSIZE), PACK(ALIGNMENT*2, 1));
	PUT(heap_listq + (2*WSIZE), NULL);
	PUT(heap_listq + (3*WSIZE), NULL);
	PUT(heap_listq + (4*WSIZE), PACK(ALIGNMENT*2, 1));
	PUT(heap_listq + (5*WSIZE), PACK(0, 1));
	heap_listq += 2*WSIZE;
	root = heap_listq;
	/* heap_listq = { 0, 9, 9, 1 } 
	   heap_listq = &haep_listq[2] */
	if (extend_heap(16) == NULL) return -1;
	return 0;
}

static void *extend_heap(size_t words)
{
	char *bp;
	size_t size;

	size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
	if (size < 2*ALIGNMENT) size = 2*ALIGNMENT;
	if ((long) (bp = mem_sbrk(size)) == -1) return NULL;

	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));
	return coalesce(bp);
}

/* 

 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	//	int newsize = ALIGN(size + SIZE_T_SIZE);
	//	void *p = mem_sbrk(newsize);
	//	if (p == (void *)-1)
	//		return NULL;
	//	else {
	//		*(size_t *)p = size;
	//		return (void *)((char *)p + SIZE_T_SIZE);
	size_t	asize;
	size_t	extendsize;
	char	*bp;

	if	(size == 0) return NULL;
	if	(heap_listq == NULL) mm_init();

	if	(size <= ALIGNMENT) asize = 2*ALIGNMENT;
	else asize = ALIGNMENT * ((size + (ALIGNMENT) + (ALIGNMENT - 1)) / ALIGNMENT);

	if ((bp = find_fit(asize)) != NULL) {
		place(bp, asize);
		return bp;
	}

	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extend_heap(extendsize/WSIZE)) == NULL) return NULL;
	place(bp, asize);
	return bp;
}

static void place(void *bp, size_t asize) {
	size_t	csize = GET_SIZE(HDRP(bp));
	
	remove_block(bp);
	if ((csize - asize) >= (2*ALIGNMENT)) {
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		bp = NEXT_BLKP(bp);
		PUT(HDRP(bp), PACK((csize - asize), 0));
		PUT(FTRP(bp), PACK((csize - asize), 0));
		//last block coalescing.
		coalesce(bp);
	}
	else{
		PUT(HDRP(bp), PACK(csize, 1));
		PUT(FTRP(bp), PACK(csize, 1));
	}
}
static void *find_fit(size_t size){
	void	*bp;
	for (bp = root; GET_ALLOC(HDRP(bp)) == 0; bp = NEXTP(bp)){
		if	((size <= GET_SIZE(HDRP(bp)))){
			return bp;
		}
	}
	return NULL;
}
// insert block to free_list.
//static void *find_fit(size_t asize){
//  void *bp;
//  static int last_malloced_size = 0;
//  static int repeat_counter = 0;
//  if( last_malloced_size == (int)asize){
//      if(repeat_counter>30){
//        int extendsize = MAX(asize, 4 * WSIZE);
//        bp = extend_heap(extendsize/4);
//        return bp;
//      }
//      else
//        repeat_counter++;
//  }
//  else
//    repeat_counter = 0;
//  for (bp = root; GET_ALLOC(HDRP(bp)) == 0; bp = NEXTP(bp) ){
//    if (asize <= (size_t)GET_SIZE(HDRP(bp)) ) {
//      last_malloced_size = asize;
//      return bp;
//    }
//  }
//  return NULL;
//}
///best_fit.
//static void *find_fit(size_t size){
//	void	*bp;
//	void	*last = NULL;
//	size_t	min = 1e9;
//	size_t	gap;
//	for (bp = root; GET_ALLOC(HDRP(bp)) == 0; bp = NEXTP(bp)){
//		gap = GET_SIZE(HDRP(bp)) - size;
//		if (gap == 0) {
//			return bp;
//		}
//		else if (0 < gap && gap < min) {
//			min = gap; 
//			last = bp;
//		} 
//	}
//	return last;
//}
void put_block(char *bp){
	NEXTP(bp) = root;
	PREVP(bp) = NULL;
	PREVP(root) = bp;
	root = bp;
}
// remove block from free_list.
void remove_block(char *bp){
	if (bp == root){
		root = NEXTP(bp);
		PREVP(NEXTP(bp)) = NULL;
	}
	else{
		NEXTP(PREVP(bp)) = NEXTP(bp);
		PREVP(NEXTP(bp)) = PREVP(bp);
	}
}
/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
	size_t size = GET_SIZE(HDRP(bp));
	// size_t size = SIZE_T_SIZE(bp);
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	coalesce(bp);
}

static void *coalesce(void *bp)
{
	//size_t	prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp;
	size_t	prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t	next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t	size = GET_SIZE(HDRP(bp));
	if	(prev_alloc && next_alloc) {
		put_block(bp);
		return bp;
	}
	if	(prev_alloc && !next_alloc) {
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		remove_block(NEXT_BLKP(bp));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	else if	(!prev_alloc && next_alloc) {
		bp = PREV_BLKP(bp);
		size += GET_SIZE(HDRP(bp));
		remove_block(bp);
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	else if (!prev_alloc && !next_alloc){
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
		remove_block(PREV_BLKP(bp));
		remove_block(NEXT_BLKP(bp));
		bp = PREV_BLKP(bp);
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	put_block(bp);
	return bp;
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
	void *oldptr = ptr;
	void *newptr;
	size_t copySize;
	//if ((int)size < 0) return NULL;
	if((int)size == 0) {mm_free(ptr); return NULL;}
	newptr = mm_malloc(size);
	if (newptr == NULL)
		return NULL;
	copySize = GET_SIZE(HDRP(oldptr)) - ALIGNMENT;
	if (size < copySize)
		copySize = size;
	memcpy(newptr, oldptr, copySize);
	mm_free(oldptr);
	return newptr;
}
