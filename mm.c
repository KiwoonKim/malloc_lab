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

typedef unsigned int list_t;
/* single word (4) or double word (8) alignment */
#define WSIZE		4
#define ALIGNMENT	8
#define CHUNKSIZE	(1<<16)
#define MIN_POWERED 3
#define MAX_POWERED 16
#define LIST_LENGTH MAX_POWERED - MIN_POWERED + 1

# define MAX(x , y) ((x) > (y)? (x) : (y))

/* rounds up to the nearest multiple of ALIGNMENT */

// read & write a word at addr p
#define GET(p)		(*(list_t*)(p))
#define GET_SIZE(p) (*(size_t*)(p))
#define PUT(p, val)	(*(unsigned int*)(p) = (val))
#define PUT_SIZE(p, val) (*(size_t*)(p) = (val))
// read size and alloc fields from addr p
#define POW_SIZE(pow)		(1 << (pow+MIN_POWERED))
#define HDRP(p)				((void *)(p))
// for decplicit lsist
// given block ptr bp, compute addr of next and previous blocks


/* 
 * mm_init - initialize the malloc package.
 */
static char	*heap_listq = NULL;
static char *heap_start = NULL;
// for explicit list
int		mm_init(void);
static void	put_block(list_t** root, list_t** pos);
static void	*extend_heap(size_t size);
static void place(char *bp, size_t f_pow, size_t a_pow);
static void *find_fit(size_t size);
static void	remove_block(list_t** root, list_t **target);

int mm_init(void)
{
	if ((heap_listq = mem_sbrk(MAX_POWERED*WSIZE)) == NULL) return -1;
	PUT_SIZE(heap_listq, 0);
	for (int i = 1; i < LIST_LENGTH + 1; i++){
		PUT(heap_listq + (i*WSIZE), NULL);
	}
	PUT_SIZE(heap_listq + (LIST_LENGTH + 1) * WSIZE, 1);
	heap_listq += WSIZE;
	heap_start = heap_listq + ((LIST_LENGTH + 1) * WSIZE);
//	printf("heap_listq start : %p, end : %p\n", heap_listq, heap_start );
	/* heap_listq = { 0, 9, 9, 1 } 
	   heap_listq = &haep_listq[2] */
	if (extend_heap(CHUNKSIZE) == NULL) return -1;
	return 0;
}

static void *extend_heap(size_t size)
{
	printf("----------------extend heap---------------\n");
	list_t *bp;
	list_t **max_free_list;
	if ((long) (bp = mem_sbrk(size)) == -1) return NULL;
	max_free_list = heap_listq + (MAX_POWERED - MIN_POWERED) * WSIZE;
	printf("max_free_list : %p, bp : %p \n", max_free_list, bp);
	put_block(max_free_list, bp);
	return bp;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	printf("------------start malloc-------------\n");
	printf("size %d\n", size);

	char *bp;
	bp = find_fit(size);
//	printf("###########end malloc###############\n");	
//	printf("in extend_heap tmp : %p\n", bp);
	return bp + ALIGNMENT;
}
// 할당 할 수있는 가용블럭중 제일 작은 블럭을 찾고, 그 과정에서 남은 블럭을 가용리스트에 넣고, 찾은 제일 작은 블럭을 할당함.
//static void *place(char *bp, size_t f_pow, size_t a_pow) {
//    
//	}
// 
static void *find_fit(size_t size){
	//	printf("------------find fit-------------\n");
	char	*root;
	int k = 0;
	while (k < MAX_POWERED) {
		if (1 << (k + MIN_POWERED) >= size) break;
		k++;
	}
//	printf("in find_fit k: %d\n",k );
	int i = k;
	root = heap_listq + (k)* WSIZE;
	//	printf("in find fit  k: %d\n", k);
	while (*(unsigned int*)(root)== NULL && *root != 1) {
		root +=  WSIZE;
		i++;
//		printf("in find_root: %p and root %p\n", *(unsigned int*)root, root);
//		printf("in find_i : %d\n", i);
	}
//	printf("in find fit i ; %d\n", i);
	//	printf("in find fit bp: %p\n", bp);
	if (*root == 1)
	{
		printf("check extend");
		extend_heap(CHUNKSIZE);
		i -= 1;
		root -= WSIZE;
	}	
	//	printf("in find_fit bp : %p\n", bp);
	char *cur = *(unsigned int*) root;
	printf("in find fit root : %p value of root : %p, cur : %p  isize : %d ksize : %d\n", root, *(unsigned int*)root, cur, POW_SIZE(i), POW_SIZE(k));
	remove_block((list_t*)root, (list_t*)cur);
	while (i > k) {
		i -= 1;
		root = root - WSIZE;
		put_block(root, cur+POW_SIZE(i)); // return bp의 값을 
	}
	PUT_SIZE(HDRP(cur), POW_SIZE(i));
//	printf("in find fit pow_size %d\n", POW_SIZE(i));
//	printf("in find fit HEADER : %p\n", cur);
//	printf("in find fit HDGET_SIZE %d\ni", GET_SIZE(HDRP(cur)));
	return cur;
	//place(bp, i, k);
}

static void put_block(list_t** root, list_t** new){
	
	printf("-------put block-------\n");
	printf("in root : %p\n", root);
	printf("root value: %p\n", *root);
	printf("in new  : %p\n", new);
	printf("-----------------------\n");
	if ((*root!=NULL))
	{	
		*new = *root;
		*((*root) + 1) = new;
		*root = new;
		//*(*(root)+1) = new; // 원래 맨 앞 원소의 prev에 new node 주소를 넣는다,
		//*(new+1) = root; // new node의prev에 root 주소를 넣는다
		//*new = *root; // new node의 next에 원래 맨 앞 원소의 주소를 넣는다
		//*root = new; // root에 new 노드 주소를 넣는다 
	}
	else{
		printf("no next!\n");
		*root = new;
		*new = NULL;
	}

	printf("root next : %p\n",*root);
	printf("new add : %p\n", new);
	printf("new next: %p\n", *new);
	printf("root next next : %p\n",**root); 
	//if (h_list){
	//	h_list->prev = pos;
	//}
	//h_list = pos;
	//buddybuddy(pos, f_pow);
}
static void remove_block(list_t** root, list_t** target){
	printf("--------------remove_block----------\n");
	printf("free list root: %p\n", root);
	printf("root value %p, target %p\n",*root,target);	
	printf("root valup %p, *target %p\n",*root,*(unsigned int*)target);	
	printf("------------------------------------\n");
	// target의 prev를 target의 next와 연결
	
	if (*target != NULL)	{
		*(*(target+1)) = *target;
		*(*(target)+1) = *(target +1); // target의 next를 target의 prev와 연결
	}
	else if (*(target + 1) == root)
	{
		*root = (*target);
	}
	else *(root)= NULL;
	printf("root value %p\n", *root);
}

static void buddybuddy(char* pos, int f_pow){
		printf("------buddy~buddy~----------\n");
	//	printf("power : %d\n", GET_SIZE(HDRP(pos))); 
	size_t pos_rel = (size_t)(pos - heap_start);
	//	printf("relative position : %p\n", pos_rel);
	size_t buddy_rel = (pos_rel ^ (1 << f_pow));
	//	printf("absolute buddy pos: %p\n", buddy_rel + heap_start);
	//	printf("buddy ret : %p\n" ,buddy_rel);
	//	printf("out buddy  while POW : %d,  HD : %d\n", 1 << f_pow ,GET_SIZE(HDRP(buddy_rel + heap_start)));
	while (!GET_SIZE(HDRP(buddy_rel + heap_start)) && f_pow < MAX_POWERED)
	{
		printf(" in buddy while POW : %d,  HD : %d\n", 1 << f_pow  ,GET_SIZE(HDRP(buddy_rel + heap_start)));
		printf(" in buddy while buddy_rel : %p, 루트주소 : %p, 루트 다음 : %p\n", buddy_rel, (list_t *)(heap_listq + (f_pow - MIN_POWERED) * WSIZE), *(list_t *)(heap_listq + (f_pow-MIN_POWERED)*WSIZE)); 	
		remove_block((list_t **)(heap_listq + (f_pow - MIN_POWERED)*WSIZE), (list_t *)(heap_start + buddy_rel));
		f_pow++;
		if ((long)buddy_rel < (long)pos_rel)
			pos_rel = buddy_rel;
		buddy_rel = (pos_rel ^ (1 << f_pow));
	}
	//	printf("while after f_pow : %d\n", f_pow);
	printf("while after root : %p\n", (list_t*)(heap_listq + ((f_pow-MIN_POWERED) * WSIZE)));
	printf("while after cur_list : %p\n", (list_t*)(buddy_rel + heap_start));
	put_block((list_t*)(heap_listq + ((f_pow - MIN_POWERED) * WSIZE)), (list_t*)(pos_rel + heap_start));
}

void mm_free(void *bp)
{
	printf("------------------free------------------\n");
	size_t size = GET_SIZE(HDRP(bp - ALIGNMENT));
	printf("mm free bp: %d\n", GET_SIZE(HDRP(bp - ALIGNMENT)));
	//	printf("mm free HEAD bp: %p\n", bp - ALIGNMENT);
//	printf("size in free : %d\n", size);
	int f_pow = 0;
	while (size > 1){
		f_pow += 1;
		size >>= 1;
	}
	//	printf("f_pow in free f_pow : %d POWERED f_pow : %d\n", f_pow, 1 << f_pow);
	buddybuddy(bp - ALIGNMENT, f_pow);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
	return ptr;
}
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
