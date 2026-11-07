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
#include <stdint.h>

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "homelander",
    /* First member's full name */
    "Stone",
    /* First member's email address */
    "I can't tell you",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* 初始指向的prologue footer第一个字节，因为payload大小为0，正常状态下指向payload的第一个字节*/
static char *heap_lisp;

/* ==================== 常量宏 ==================== */

/* 对齐要求，单位：字节 */
#define ALIGNMENT 8

/* 字大小与双字大小，单位：字节 */
#define W_SIZE 4
#define D_SIZE 8

/* 默认堆扩展大小：4 KiB，根据教材上设置 */
#define CHUNK_SIZE (1 << 12)

/* 原始分配器的头部预留大小，按 ALIGNMENT 对齐 */
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* ==================== 操作宏 ==================== */

/* 对齐计算：向上取整到 ALIGNMENT 的整数倍 */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

/* 在对应字放置数值*/
#define PUT(p, val) (*(uint32_t *)(p) = (val)) 
/* 获取一个字的val*/
#define GET(p) (*(uint32_t *)(p)) 

/* 指针指向payload的首字节，获取当前块header指针*/
#define HDRP(bp) ((char *)(bp) - W_SIZE)

/* 指针指向payload的首字节，获取当前块foot指针*/
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - D_SIZE)

/* 获取头部的size大小，和分配情况 */
#define GET_SIZE(p) (GET(p) & ~0x7) // 0x7低三位是1，取反就是只有低三位是0
#define GET_ALLOC(p) (GET(p) & 0x1)

/* 找到上一个或下一个块的payload指针*/
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - D_SIZE))

/* 取最大值，不要传入i++等类似的形式*/
#define MAX(a, b) ((a) > (b)? (a):(b))

/**
 * 将块总大小（字节数，包含头尾）和分配状态打包到一个字中。
 * [4 字节 header][至少 8 字节 payload][4 字节 footer]
 * 块总大小是 8 的倍数，其低三位为 0，因此可用最低位记录状态：
 * 1 表示已分配，0 表示空闲。
 */
#define PACK(size, alloc) ((size) | (alloc))

/* ==================== 函数声明 ==================== */

/* 对空闲块进行合并 */
static void *coalesce(void *bp);
/* 拓展堆words个字（word数），保持对齐；返回合并后空闲块的 payload 指针，失败返回 NULL */
static void *extend_heap(size_t words);
/* 按照顺序找到足够大小的块*/
static void *findfit(size_t size);
/* 将足够大小的块进行分割，最小为2*D_SIZE（header + footer + payload(对齐最小为D_SIZE)）*/
static void place(void *bp, size_t size);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // 1 扩展堆四个Word size，，word[1]放置prologue header，word[2]
    if((heap_lisp = mem_sbrk(4 * W_SIZE)) == NULL){
        return -1; // -1代表错误
    }

    // 2 word[0]对齐,word[1]作为prologue header，word[2]作为prologue footer，word[3]作为Epilogue head，末尾只有header
    PUT(heap_lisp, 0);
    PUT(heap_lisp + W_SIZE, PACK(D_SIZE, 1));
    PUT(heap_lisp + 2*W_SIZE, PACK(D_SIZE, 1));
    PUT(heap_lisp + 3*W_SIZE , PACK(W_SIZE, 1));

    heap_lisp += (2 * W_SIZE);

    // 3 给栈一个初始化空间
    if(extend_heap(CHUNK_SIZE/W_SIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 * 
 * 这里的size是payload的大小
 */
void *mm_malloc(size_t size)
{
    size_t all_size;
    size_t extend_size; // 如果说不满足，需要拓展的大小
    char *bp;

    // 1 将size进行字节数对齐，并且保留header和footer的size
    if(size == 0){
        return NULL;
    }
    if(size < D_SIZE){
        all_size = 2*D_SIZE; // header+footer+对齐
    }else{
        all_size = D_SIZE + ALIGN(size);
    }

    // 2 使用某个策略找到合适的空闲片
    if((bp = findfit(all_size)) != NULL){
        place(bp, all_size); // 分割块
        return bp;
    }

    // 3 没有合适的块，扩展堆
    extend_size = MAX(CHUNK_SIZE, all_size);
    if((bp = extend_heap(extend_size/W_SIZE)) == NULL)
        return NULL;

    place(bp, all_size); // 分割块
    return bp;
}

/** 
 * mm_free - Freeing a block does nothing.
 * 标记为未分配并前后合并
 */
void mm_free(void *ptr)
{
    // 1 将头尾标记为未分配
    size_t asize = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(asize, 0));
    PUT(FTRP(ptr), PACK(asize, 0));

    // 2 合并
    coalesce(ptr);
}

/**
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 * 把 ptr 指向的已分配空间调整为 size 字节，返回调整后的指针。这里 size 是用户数据大小，不包含头尾
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/* 拓展堆words个字（word数），保持对齐；返回合并后空闲块的 payload 指针，失败返回 NULL */
static void *extend_heap(size_t words)
{
    char* bp;
    size_t size; // 真正对齐后的字节数

    size = words * W_SIZE;
    size = ALIGN(size);
    // 1 增加栈words个字
    /* 失败时直接比较指针 */
    if ((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;

    // 2 头尾部赋值，这也是为啥尾部只需要一个header，方便向前获取
    uint32_t pack_temp = PACK(size, 0);
    PUT(HDRP(bp), pack_temp);
    PUT(FTRP(bp), pack_temp);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // Epilogue head

    // 3 合并
    return coalesce(bp); // 这里其实只可能合并前项，mem_sbrk是末尾扩
}

/* 对空闲块进行合并 */
static void *coalesce(void *bp)
{
    // 1 获取前后块的信息
    char *front_bp = PREV_BLKP(bp);
    char *back_bp = NEXT_BLKP(bp);
    size_t front_size = GET_SIZE(HDRP(front_bp));
    size_t back_size = GET_SIZE(HDRP(back_bp));
    size_t front_alloc = GET_ALLOC(HDRP(front_bp));
    size_t back_alloc = GET_ALLOC(HDRP(back_bp));

    // 2 case1：前面块和后面块都被分配，啥都不用干
    if(front_alloc && back_alloc){
        return bp;
    }else if(!front_alloc && back_alloc){
    // 3 case2：前面块空闲，后面块分配
        size_t size = front_size + GET_SIZE(HDRP(bp));
        PUT(HDRP(front_bp), PACK(size, 0));
        PUT(FTRP(front_bp), PACK(size, 0)); // FTRP是由HDRP计算出来的，可以直接用front_bp
        bp = front_bp;
    }else if(front_alloc && !back_alloc){
    // 4 case3：前面块分配，后面块空闲
        size_t size = back_size + GET_SIZE(HDRP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }else{
    // 5 case4：前后块都空闲
        size_t size = back_size + front_size + GET_SIZE(HDRP(bp));
        PUT(HDRP(front_bp), PACK(size, 0));
        PUT(FTRP(front_bp), PACK(size, 0)); // FTRP是由HDRP计算出来的，可以直接用front_bp
        bp = front_bp;
    }
    
    return bp; 
}

/* 按照顺序找到足够大小的块*/
static void *findfit(size_t size)
{
    // 这里直接按照顺序查从heap_lisp开始找到第一个
    char *bp;

    for ( bp = heap_lisp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if(!GET_ALLOC(HDRP(bp)) && (size <= GET_SIZE(HDRP(bp)))){
            return bp;
        }
    }
    
    return NULL;
}


/* 将足够大小的块进行分割，最小为2*D_SIZE（header + footer + payload(对齐最小为D_SIZE)）*/
static void place(void *bp, size_t size)
{
    // 1 判断当前块大小
    size_t b_size = GET_SIZE(HDRP(bp));
    
    // 2 差值大于等于 2*D_SIZE可分割
    if((b_size - size) >= (2*D_SIZE)){
        size_t remaining_size = b_size - size;
        // 更改当前块header、footer
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        // 更改下块header、footer
        PUT(HDRP(NEXT_BLKP(bp)), PACK(remaining_size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(remaining_size, 0));
    }else{
    // 3 反之不可分割
        PUT(HDRP(bp), PACK(b_size, 1));
        PUT(FTRP(bp), PACK(b_size, 1));     
    }
}






