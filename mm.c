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

typedef struct list_t{
	struct list_t *prev, *next;
} list_t;
/* single word (4) or double word (8) alignment */
#define WSIZE		4
#define ALIGNMENT	8
#define CHUNKSIZE	(1<<16)
#define MIN_POWERED 3
#define MAX_POWERED 16

# define MAX(x , y) ((x) > (y)? (x) : (y))

/* rounds up to the nearest multiple of ALIGNMENT */

// read & write a word at addr p
#define GET(p)		(*(list_t*)(p))
#define GET_SIZE(p) (*(size_t*)(p))
#define PUT(p, val)	(*(unsigned int*)(p) = (val))
#define PUT_SIZE(p, val) (*(size_t*)(p) = (val))
// read size and alloc fields from addr p
#define POW_SIZE(pow)		(1 << (pow+MIN_POWERED))
#define HDRP(p)				((void *)(p)-WSIZE)
// for decplicit lsist
// given block ptr bp, compute addr of next and previous blocks

/* 
 * mm_init - initialize the malloc package.
 */
static char	*heap_listq = NULL;
static char *heap_start = NULL;
// for explicit list
int		mm_init(void);
void	put_block(list_t* h_list, list_t* pos, int pow);

static void	*extend_heap(size_t size);
static void place(char *bp, size_t f_pow, size_t a_pow);
static void *find_fit(size_t size);

int mm_init(void)
{
	if ((heap_listq = mem_sbrk(MAX_POWERED*WSIZE)) == NULL) return -1;
	PUT_SIZE(heap_listq, 0);
	for (int i = 1; i < MAX_POWERED - MIN_POWERED + 1; i++){
		PUT(heap_listq + (i*WSIZE), NULL);
	}
	PUT_SIZE(heap_listq + (MAX_POWERED - MIN_POWERED + 1) * WSIZE, 1);
	heap_start = heap_listq + (MAX_POWERED*WSIZE);
	/* heap_listq = { 0, 9, 9, 1 } 
	   heap_listq = &haep_listq[2] */
	if (extend_heap(CHUNKSIZE) == NULL) return -1;
	return 0;
}

static void *extend_heap(size_t size)
{
	list_t *bp;
	if ((long) (bp = mem_sbrk(size)) == -1) return NULL;
	list_t start_buddy;
	*bp = start_buddy;
	bp->prev = (list_t *) (heap_listq + MAX_POWERED - MIN_POWERED);
	bp->next = NULL;
	heap_listq[9] = bp;
	return bp;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	find_fit(size);

	return NULL;
}

static void place(char *bp, size_t f_pow, size_t a_pow) {
    
	char *cur = *bp;
	remove_block(bp);
	while (f_pow > a_pow) {
		f_pow -= 1;
		bp = bp - WSIZE;
		put_block(bp, cur+POW_SIZE(f_pow), f_pow); // return bp의 값을 
	}
	PUT_SIZE(HDRP(cur), POW_SIZE(f_pow));
}

static void *find_fit(size_t size){
	char	*bp;
	size_t	asize;
	int k = 0;
	while (k < MAX_POWERED) {
		if (1 << (k + MIN_POWERED) >= size) break;
		k++;
	}
	asize = 1<<(k+MIN_POWERED);
	int i = k;
	while (*bp == NULL || *bp != 1) {
		bp = heap_listq + (i + 1) * WSIZE;
		i++;
	}
	if (*bp == 1)
	{
		extend_heap(CHUNKSIZE);
		i -= 1;
		bp -= WSIZE;
	}
	place(bp, i, k);

	return NULL;
}

static void put_block(list_t* h_list, list_t* pos, int f_pow){
	pos->next = h_list;
	pos->prev = NULL;
	if (h_list){
		h_list->prev = pos;
	}
	h_list = pos;
	//buddybuddy(pos, f_pow);
}
static void remove_block(list_t* h_list){
	h_list = h_list->next;
	h_list->prev = NULL;
}

static void buddybuddy(char* pos, int f_pow){
	size_t pos_rel = (size_t)(pos - heap_start);
	size_t buddy_rel = (pos_rel ^ pos_rel << f_pow);
	while (POW_SIZE(f_pow) == GET_SIZE(HDRP(buddy_rel + heap_start)))
	{
		remove_block((list_t *)(buddy_rel + heap_start));
		f_pow++;
		if ((long)buddy_rel < (long)pos_rel)
			pos_rel = buddy_rel;
		buddy_rel = (pos_rel ^ pos_rel << f_pow);
	}
	put_block((list_t*)(heap_listq + ((f_pow - 2) * WSIZE)), (list_t*)(buddy_rel + heap_start), POW_SIZE(f_pow));
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
//    if (asize <= (size_t)POW_SIZE(HDRP(bp)) ) {
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
//		gap = POW_SIZE(HDRP(bp)) - size;
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
void mm_free(void *bp)
{
	size_t size = GET_SIZE(HDRP(bp));
	int f_pow = 0;
	while (size > 1){
		f_pow += 1;
		size >>= 1;
	
	buddybuddy(bp, f_pow);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
// void *mm_realloc(void *ptr, size_t size)
// {
//	void *oldptr = ptr;
//	void *newptr;
//	size_t copySize;
//	//if ((int)size < 0) return NULL;
//	if((int)size == 0) {mm_free(ptr); return NULL;}
//	newptr = mm_malloc(size);
//	if (newptr == NULL)
//		return NULL;
//	copySize = POW_SIZE(HDRP(oldptr)) - ALIGNMENT;
//	if (size < copySize)
//		copySize = size;
//	memcpy(newptr, oldptr, copySize);
//	mm_free(oldptr);
//	return newptr;
// }
