#ifndef __ZSIM_HOOKS_H__
#define __ZSIM_HOOKS_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Avoid optimizing compilers moving code around this barrier
#define COMPILER_BARRIER()                    \
    {                                         \
        __asm__ __volatile__("" ::            \
                                 : "memory"); \
    }

//These need to be in sync with the simulator
#define ZSIM_MAGIC_OP_ROI_BEGIN (1025)
#define ZSIM_MAGIC_OP_ROI_END (1026)
#define ZSIM_MAGIC_OP_REGISTER_THREAD (1027)
#define ZSIM_MAGIC_OP_HEARTBEAT (1028)
#define ZSIM_MAGIC_OP_WORK_BEGIN (1029) //ubik
#define ZSIM_MAGIC_OP_WORK_END (1030)   //ubik

#define ZSIM_MAGIC_OP_FUNCTION_BEGIN (1031) // LOIS
#define ZSIM_MAGIC_OP_FUNCTION_END (1032)   // LOIS

#define ZSIM_MAGIC_OP_SET_BUFF (1033)
#define ZSIM_MAGIC_OP_LAYER_CHECKPOINT (1034)
#define ZSIM_MAGIC_OP_NOP (1035)
#define ZSIM_MAGIC_OP_LAYER_CHECKPOINT_KERNEL (1036)

#ifdef __x86_64__
#define HOOKS_STR "HOOKS"
static inline void zsim_magic_op(uint64_t op)
{
    COMPILER_BARRIER();
    __asm__ __volatile__("xchg %%rcx, %%rcx;"
                         :
                         : "c"(op));
    COMPILER_BARRIER();
}
static inline void zsim_magic_op_2(uint64_t op, uint64_t buff, uint64_t buffsz)
{
    COMPILER_BARRIER();
    __asm__ __volatile__("xchg %%rcx, %%rcx;"
                         :
                         : "c"(op), "a"(buff), "b"(buffsz));
    COMPILER_BARRIER();
}
#else
#define HOOKS_STR "NOP-HOOKS"
static inline void zsim_magic_op(uint64_t op)
{
    //NOP
}
#endif

static inline void zsim_roi_begin()
{
    printf("[" HOOKS_STR "] ROI begin\n");
    zsim_magic_op(ZSIM_MAGIC_OP_ROI_BEGIN);
}

static inline void zsim_roi_end()
{
    zsim_magic_op(ZSIM_MAGIC_OP_ROI_END);
    printf("[" HOOKS_STR "] ROI end\n");
}

// LOIS
static inline void zsim_PIM_function_begin()
{
    zsim_magic_op(ZSIM_MAGIC_OP_FUNCTION_BEGIN);
}

// LOIS
static inline void zsim_PIM_function_end()
{
    zsim_magic_op(ZSIM_MAGIC_OP_FUNCTION_END);
}

static inline void zsim_heartbeat()
{
    zsim_magic_op(ZSIM_MAGIC_OP_HEARTBEAT);
}

static inline void zsim_work_begin() { zsim_magic_op(ZSIM_MAGIC_OP_WORK_BEGIN); }
static inline void zsim_work_end() { zsim_magic_op(ZSIM_MAGIC_OP_WORK_END); }

static inline void zsim_set_buff(uint64_t buff, uint64_t buffsz, char *description)
{
    // printf("====== HOOK BUFF ~%llu~  %llu  ~%s~\n", buff, buffsz, description);

    zsim_magic_op_2(ZSIM_MAGIC_OP_SET_BUFF, buff, buffsz);
    char str[32];
    strcpy(str, description);
    zsim_magic_op_2(ZSIM_MAGIC_OP_SET_BUFF, buff, (uint64_t)str);
}

static inline void zsim_layer_checkpoint() { zsim_magic_op(ZSIM_MAGIC_OP_LAYER_CHECKPOINT); }

static inline void zsim_nop(int a, int b, int c)
{
// b = a + c;
#if 01
    zsim_magic_op(ZSIM_MAGIC_OP_NOP);
#else
    __asm__("nop ");
#endif
}

static inline void zsim_layer_checkpoint_kernel(char *description)
{
    char str[32];
    strcpy(str, description);
    zsim_magic_op_2(ZSIM_MAGIC_OP_LAYER_CHECKPOINT_KERNEL, 0, (uint64_t)str);
}

//#define SMB_WORDS 1024 / 4
// 1 KB

#endif /*__ZSIM_HOOKS_H__*/
