/* memory allocation - simple segregation free list
   1. search proper free list
   2. if the free list is, first one allocate
   3. each free blocks does not split
   4. if didn't have proper block, extends addtional heap.
   5. the heap is splited as same size, and make new free list.
   6. add it at the first of the old free list.
*/

#include <stdlib.h>
#include <string.h>

#include "memlib.h"
#include "mm.h"



#define WSIZE 4

#define CHUNCKSIZE 1<<16
#define MAX_FREELIST 12

#define PUT(a, b) *(unsigned int*)(a) = (b)


static	char*	heap_start;
static	char*	page_start;

int		mm_init();
void	*extends_heap(size_t size);
void	split_page(void *bp);

typedef enum { 8, 16, 32, 48, 64, 80, 96, 112, 128, 256, 512, 1024 } size_list;

int	mm_init(){
	if (heap_start = mem_sbrk(MAX_FREELIST*WSIZE) == (void *)-1) return -1;
	for (int i = 0; i < MAX_FREELIST; i++){
		PUT(heap_start + (i*WSIZE), NULL);
	}
	if (extends_heap(CHUNCKSIZE) == NULL) return -1;

	return 1;
}

void *extends heap(size_t size){
	void *bp;

	if (bp = mem_sbrk(size) == (*void)-1) return NULL;
	split_page(bp);

	return bp;
}

void split_page(void *bp){
	 
	for (int i = 0; i < MAX_FREELIST; i++)
	{
		for (int j = 0; j < CHUNCKSIZE / MAX_FREELIST; j++){
			PUT(heap_start + (i*WSIZE), bp + j);
		}




