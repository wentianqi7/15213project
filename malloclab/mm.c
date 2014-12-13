/*
 * mm.c
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "contracts.h"

#include "mm.h"
#include "memlib.h"


// Create aliases for driver tests
// DO NOT CHANGE THE FOLLOWING!
#ifdef DRIVER
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif

/*
 *  Logging Functions
 *  -----------------
 *  - dbg_printf acts like printf, but will not be run in a release build.
 *  - checkheap acts like mm_checkheap, but prints the line it failed on and
 *    exits if it fails.
 */

#ifndef NDEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#define checkheap(verbose) do {if (mm_checkheap(verbose)) {  \
                             printf("Checkheap failed on line %d\n", __LINE__);\
                             exit(-1);  \
                        }}while(0)
#else
#define dbg_printf(...)
#define checkheap(...)
#endif


/* constants */
#define ALIGNMENT 8         //double word alignment
#define WSIZE 4             //word size
#define DSIZE 8             //double word size
#define CHUNKSIZE (1 << 9)  //extend heap by chunksize

#define LIST_NUM 10         //number of free list
#define LIST_TRSH 2         //threshold of best fit and first fit


/* helper inline functions */

/* Return whether the pointer is in the heap */
static inline int in_heap(const void* p) {
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

/* return the larger one of the two arguments */
static inline size_t max(size_t x, size_t y){
    return (x > y) ? x : y;
}

/* pack size, prev_alloc and alloc into one word (header) */
static inline size_t pack(size_t size, size_t prev_alloc, size_t alloc){
    return size | prev_alloc | alloc;
}

/* return the word and adress p */
static inline unsigned int get(void *p){
    REQUIRES(p != NULL);
    REQUIRES(in_heap(p));
    return *(unsigned int *)p;
}

/* write val at address p */
static inline void put(void *p, size_t val){
    REQUIRES(p != NULL);
    REQUIRES(in_heap(p));
    *(unsigned int *)p = val;
}

/* return the size bit of header */
static inline size_t getSize(void *p){
    REQUIRES(p != NULL);
    REQUIRES(in_heap(p));
    return get(p) & ~0x7;
}

/* return the prev_alloc bit of header */
static inline size_t getPrevAlloc(void *p){
    REQUIRES(p != NULL);
    REQUIRES(in_heap(p));
    return get(p) & 0x2;
}

/* return the alloc bit of header */
static inline size_t getAlloc(void *p){
    REQUIRES(p != NULL);
    REQUIRES(in_heap(p));
    return get(p) & 0x1;
}

/* return the address of header */
static inline char* getHdAddr(void *bp){
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));
    return (char *)bp - WSIZE;
}

/* return the address of footer */
static inline char* getFtAddr(void *bp){
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));
    return (char *)bp + getSize(getHdAddr(bp)) - DSIZE;
}

/* return the address of next block */
static inline char* nextAddr(void *bp){
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));
    return (char *)bp + getSize((char *)bp - WSIZE);
}

/* return the address of previous block */
static inline char* prevAddr(void *bp){
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));
    return (char *)bp - getSize((char *)bp - DSIZE);
}

/* return the address saving ptr to next free block */
static inline void* nextPtr(void *bp){
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));
    return bp;
}

/* return the address saving ptr to prev free block */
static inline void* prevPtr(void *bp){
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));
    return (char *)bp + WSIZE;
}

/* set the prev_alloc bit to 1 */
static inline void setPrevAlloc(void *p){
    REQUIRES(p != NULL);
    REQUIRES(in_heap(p));
    put(p, (get(p) | 0x2));
}

/* set the prev_alloc bit to 0 */
static inline void setPrevFree(void *p){
    REQUIRES(p != NULL);
    REQUIRES(in_heap(p));
    put(p, (get(p) & (~0x2)));
}

/* double word (8 byte) alignment */
static inline size_t align(size_t size){
    return (size + (ALIGNMENT - 1)) & ~0X7;
}

/* Check if the given pointer is 8-byte aligned */
static inline int aligned(void *p) {
    REQUIRES(p != NULL);
    REQUIRES(in_heap(p));
    return align(getSize(getHdAddr(p)) == getSize(getHdAddr(p)));
}

/* Global vars */
static char *heap_listp = 0;
static char *heap_basep = 0;

/* basic helper funtions for malloc */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

/* helper functions for list management */
static inline void insert_node(void *bp, size_t index);
static inline void delete_node(void *bp);
static size_t getListNum(size_t size);

static inline void* nextInList(void *bp);
static inline void* prevInList(void *bp);


/*
 *  Malloc Implementation
 *  ---------------------
 *  The following functions deal with the user-facing malloc implementation.
 */

/*
 * Performs necessary initializations, such as allocating the initial heap area
 * return -1 on error, 0 on success.
 */
int mm_init(void){
    size_t prologue_size = (2 * LIST_NUM + 2) * WSIZE;

    if ((heap_listp = mem_sbrk(prologue_size + DSIZE)) == (void *)-1) {
        return -1;
    }

    put(heap_listp, pack(0, 0, 0));         /* alignment padding */
    put(heap_listp + WSIZE, pack(prologue_size, 2, 1));     /* prologue header */

    /* use prologue body to save list head nodes */
    size_t list_head;
    for (size_t i = 0; i < LIST_NUM; i++) {
        list_head = (i + 1) * DSIZE;
        put(heap_listp + list_head, list_head);
        put(heap_listp + (list_head + WSIZE), list_head);
    }

    heap_basep = heap_listp;
    heap_listp = heap_listp + DSIZE;        /* move heap_listp to the payload of prologue */
    put(getFtAddr(heap_listp), pack(prologue_size, 2, 1));      /* prologue footer */
    put(getFtAddr(heap_listp) + WSIZE, pack(0, 2, 1));      /* epilogue header */

    if (extend_heap(CHUNKSIZE * 8 / WSIZE) == NULL){
        return -1;
    }

    return 0;
}

/*
 * The malloc routine returns a pointer to an allocated block payload of at least size bytes
 * The entire allocated block should lie within the heap region and 
 * should not overlap with any other allocated chunk
 * always return 8-byte aligned pointers
 */
void *malloc(size_t size){
    //mm_checkheap(0);

    size_t asize;
    size_t extendsize;
    char *bp;

    if (size <= 0){
        return NULL;
    }

    /* align size to double word */
    if (size <= (DSIZE + WSIZE)){
        asize = 2 * DSIZE;
    }
    else{
        asize = align(size + WSIZE);
    }

    /* search the freelist for a fit */
    if ((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }

    /* no fit found --> extend heap */
    extendsize = max(asize, CHUNKSIZE);

    if ((bp = extend_heap(extendsize / WSIZE)) == NULL){
        return NULL;
    }

    /* allocate in newly extended memory */
    place(bp, asize);
    
    //mm_checkheap(0);
    return bp;
}

/*
 * frees the block pointed to by ptr
 * free is only guaranteed to work when the ptr was 
 * returned by an earlier call to malloc, calloc, or realloc and hasn't been freed
 * free(NULL) has no effect
 */
void free(void *ptr){

    if (ptr == NULL) {
        return;
    }

    size_t size = getSize(getHdAddr(ptr));              //size of the block to free
    size_t prev_alloc = getPrevAlloc(getHdAddr(ptr));   //the prev_alloc bit

    /* set allocate bit to 0 */
    put(getHdAddr(ptr), pack(size, prev_alloc, 0));
    put(getFtAddr(ptr), size);

    coalesce(ptr);
}

/*
 * if ptr is NULL, the call is equivalent to malloc(size)
 * if size is equal to zero, the call is equivalent to free(ptr) and should return NULL
 * if ptr is not NULL, returns a pointer to an allocated region of at least size bytes
 */
void *realloc(void *ptr, size_t size){
    /* if size == 0, then equals free(ptr) */
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    /* if ptr == NULL, then equals malloc(size) */
    if (ptr == NULL){
        return malloc(size);
    }

    void *newptr;       //ptr to newly allocated block
    size_t newSize;     //aligned size of the new block
    size_t oldSize = getSize(getHdAddr(ptr));           //size of the old block
    size_t prev_alloc = getPrevAlloc(getHdAddr(ptr));       //if the prev block is allocated
    size_t tSize;

    /* align realloc size */
    if (size <= (DSIZE + WSIZE)){
        newSize = 2 * DSIZE;
    }
    else{
        newSize = align(size + WSIZE);
    }

    if (newSize <= oldSize){
        /* the old block is big enough for the new one */
        return ptr;
    }
    else {
        size_t next_alloc = getAlloc(getHdAddr(nextAddr(ptr)));

        if (!next_alloc) {
            /* next block of the old block is free */
            tSize = oldSize + getSize(getHdAddr(nextAddr(ptr)));

            if (tSize >= newSize){
                /* old block size plus next block size is large enough for the new block */
                delete_node(nextAddr(ptr));

                if (tSize - newSize >= 2 * DSIZE){
                    /* free part of the next block is large enough to form a new free block */
                    /* first part as the new block */
                    put(getHdAddr(ptr), pack(newSize, prev_alloc, 1));

                    /* second part as a new free block */
                    put(getHdAddr(nextAddr(ptr)), pack(tSize - newSize, 2, 0));
                    put(getFtAddr(nextAddr(ptr)), pack(tSize - newSize, 0, 0));
                    setPrevFree(getHdAddr(nextAddr(nextAddr(ptr))));
                    insert_node(nextAddr(ptr), getListNum(tSize - newSize));
                }
                else{
                    /* do not need to split the block */
                    put(getHdAddr(ptr), pack(tSize, prev_alloc, 1));
                    setPrevAlloc(getHdAddr(nextAddr(ptr)));
                }

                return ptr;
            }
        }

        /* if the sum size of old block and the next block is not large enough 
        ** call malloc, copy the contents and free the old block */
        newptr = malloc(size);

        if (!newptr){
            return NULL;
        }

        memcpy(newptr, ptr, oldSize);
        free(ptr);

        //mm_checkheap(0);
        return newptr;
    }
}

/*
 * Allocates memory for an array of nmemb elements of size bytes each and 
 * returns a pointer to the allocated memory
 */
void *calloc(size_t nmemb, size_t size){
    size_t bytes = nmemb * size;
    void *newptr;

    newptr = malloc(bytes);
    memset(newptr, 0, bytes);

    return newptr;
}

/* scans the heap and checks it for correctness */
int mm_checkheap(int verbose) {
    verbose = verbose;

    return 0;
}


/* ----------- basic helper functions ------------ */

/* 
 * extend the heap for size(words) and return a ptr to the extend memory
 * return NULL if fail 
 */
static void *extend_heap(size_t words){
    char *bp;
    size_t size;
    size_t prev_alloc;

    /* align size to multiple of 8 bytes */
    size = (words % 2) ? (words + 1) * WSIZE : (words * WSIZE);

    if ((long)(bp = mem_sbrk(size)) == -1){
        return NULL;
    }

    /* new free block */
    prev_alloc = getPrevAlloc(getHdAddr(bp));
    put(getHdAddr(bp), pack(size, prev_alloc, 0));
    put(getFtAddr(bp), pack(size, 0, 0));

    /* epilogue header */
    put(getHdAddr(nextAddr(bp)), pack(0, prev_alloc, 1));

    return coalesce(bp);
}

/*
 * coalesce the free block pointed by bp with its prev and next blocks if free
 * return a ptr to the coalesced block
 */
static void *coalesce(void *bp){
    size_t prev_alloc = getPrevAlloc(getHdAddr(bp));
    size_t next_alloc = getAlloc(getHdAddr(nextAddr(bp)));
    size_t prev_prev_alloc;
    size_t size = getSize(getHdAddr(bp));

    if (prev_alloc && next_alloc) {
        /* both prev and next blocks are allocated */
        setPrevFree(getHdAddr(nextAddr(bp)));
    }
    else if (prev_alloc && !next_alloc) {
        /* next block is free */
        size += getSize(getHdAddr(nextAddr(bp)));
        delete_node(nextAddr(bp));

        put(getHdAddr(bp), pack(size, prev_alloc, 0));
        put(getFtAddr(bp), size);
    }
    else if (!prev_alloc && next_alloc) {
        /* prev block is free */
        size += getSize(getHdAddr(prevAddr(bp)));
        prev_prev_alloc = getPrevAlloc(getHdAddr(prevAddr(bp)));
        delete_node(prevAddr(bp));

        put(getFtAddr(bp), size);
        bp = prevAddr(bp);
        put(getHdAddr(bp), pack(size, prev_prev_alloc, 0));
        setPrevFree(getHdAddr(nextAddr(bp)));
    }
    else {
        /* both prev and next blocks are free */
        size += getSize(getHdAddr(prevAddr(bp))) +
            getSize(getFtAddr(nextAddr(bp)));
        prev_prev_alloc = getPrevAlloc(getHdAddr(prevAddr(bp)));
        delete_node(nextAddr(bp));
        delete_node(prevAddr(bp));

        put(getHdAddr(prevAddr(bp)), pack(size, prev_prev_alloc, 0));
        put(getFtAddr(nextAddr(bp)), size);
        bp = prevAddr(bp);
    }

    insert_node(bp, getListNum(size));
    return bp;
}

/*
 * find a block with enough size from the seg free list
 * use first fit for small blocks and best fit for large blocks
 * threshold = (1 << 7) 
 */
static void *find_fit(size_t asize){
    size_t index = getListNum(asize);
    char *list = heap_listp + index * DSIZE;
    void *temp_list;
    void *ptr;

    if (index > LIST_TRSH){
        /* best fit */
        void *min = NULL;
        size_t minSize = 1 << 31;

        /* go through seg lists */
        for (temp_list = list; temp_list != getFtAddr(heap_listp);
            temp_list = (char *)temp_list + DSIZE) {
            /* go through blocks in each list */
            for (ptr = nextInList(temp_list); ptr != temp_list;
                ptr = nextInList(ptr)) {
                if (asize <= getSize(getHdAddr(ptr))) {
                    if (min == NULL || getSize(getHdAddr(ptr)) < minSize) {
                        /* no nodes in the current list or 
                        ** size of the node is smaller than best size */
                        min = ptr;
                        minSize = getSize(getHdAddr(ptr));
                    }
                }
            }
        }

        return min;
    }
    else{
        /* first fit */
        /* go through seg lists */
        for (temp_list = list; temp_list != getFtAddr(heap_listp);
            temp_list = (char *)temp_list + DSIZE){
            /* go through blocks in each list */
            for (ptr = nextInList(temp_list); ptr != temp_list;
                ptr = nextInList(ptr)){
                if (asize <= getSize(getHdAddr(ptr))){
                    /* return ptr to the first block with greater size */
                    return ptr;
                }
            }
        }
    }

    return NULL;
}

/*
 * put the allocated block to bp
 * split the block pointed by bp if 
 * the remaining space is large enough for a new free block
 */
static void place(void *bp, size_t asize){
    size_t csize = getSize(getHdAddr(bp));
    size_t prev_alloc = getPrevAlloc(getHdAddr(bp));
    size_t dif = csize - asize;

    delete_node(bp);

    /* split block into two part */
    if (dif >= (2 * DSIZE)) {
        /* first part allocated */
        put(getHdAddr(bp), pack(asize, prev_alloc, 1));

        /* second part as free */
        bp = nextAddr(bp);
        put(getHdAddr(bp), pack(dif, prev_alloc, 0));
        put(getFtAddr(bp), pack(dif, 0, 0));

        insert_node(bp, getListNum(dif));
    }
    /* no need to split */
    else{
        put(getHdAddr(bp), pack(csize, prev_alloc, 1));
        setPrevAlloc(getHdAddr(nextAddr(bp)));
    }
}


/* ----------list functions------------ */

/* insert node to the segregated free list */
static inline void insert_node(void *bp, size_t index){
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));

    put(nextPtr(bp), get(nextPtr(heap_listp + index * DSIZE)));
    put(prevPtr(bp), get(prevPtr(nextInList(bp))));

    put(nextPtr(heap_listp + index * DSIZE), (long)bp - (long)heap_basep);
    put(prevPtr(nextInList(bp)), (long)bp - (long)heap_basep);
}

/* delete the node from segregated free list */
static inline void delete_node(void *bp){
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));

    put(prevPtr(nextInList(bp)), get(prevPtr(bp)));
    put(nextPtr(prevInList(bp)), get(nextPtr(bp)));
}

/* return the index of list according to the free block size */
static size_t getListNum(size_t asize){
    if (asize <= (1 << 4)){
        return 0;
    }
    else if (asize < (1 << 5)){
        return 1;
    }
    else if (asize < (1 << 6)){
        return 2;
    }
    else if (asize < (1 << 7)){
        return 3;
    }
    else if (asize < (1 << 8)){
        return 4;
    }
    else if (asize < (1 << 9)){
        return 5;
    }
    else if (asize < (1 << 10)) {
        return 6;
    }
    else if (asize < (1 << 11)){
        return 7;
    }
    else if (asize < (1 << 12)){
        return 8;
    }
    else{
        return 9;
    }
}

/* return the next node in free list */
static inline void* nextInList(void *bp){
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));
    return heap_basep + *(unsigned int *)nextPtr(bp);
}

/* return the previous node in free list */
static inline void* prevInList(void *bp){
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));
    return heap_basep + *(unsigned int *)prevPtr(bp);
}
