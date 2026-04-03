#ifndef __MINTOMIC_PRIVATE_MINTOMIC_GCC_ARM64_H__
#define __MINTOMIC_PRIVATE_MINTOMIC_GCC_ARM64_H__

#ifdef __cplusplus
extern "C" {
#endif


//-------------------------------------
//  Atomic types
//-------------------------------------
#if MINT_HAS_C11_MEMORY_MODEL
    typedef struct { uint32_t _nonatomic; } __attribute__((aligned(4))) mint_atomic32_t;
    typedef struct { uint64_t _nonatomic; } __attribute__((aligned(8))) mint_atomic64_t;
    typedef struct { void *_nonatomic; } __attribute__((aligned(8))) mint_atomicPtr_t;
#else
    typedef struct { volatile uint32_t _nonatomic; } __attribute__((aligned(4))) mint_atomic32_t;
    typedef struct { volatile uint64_t _nonatomic; } __attribute__((aligned(8))) mint_atomic64_t;
    typedef struct { void *volatile _nonatomic; } __attribute__((aligned(8))) mint_atomicPtr_t;
#endif


//-------------------------------------
//  Fences
//-------------------------------------
#define mint_signal_fence_consume() (0)
#define mint_signal_fence_acquire() __asm__ __volatile__("" ::: "memory")
#define mint_signal_fence_release() __asm__ __volatile__("" ::: "memory")
#define mint_signal_fence_seq_cst() __asm__ __volatile__("" ::: "memory")

#define mint_thread_fence_consume() (0)
#define mint_thread_fence_acquire() __asm__ __volatile__("dmb ishld" ::: "memory")
#define mint_thread_fence_release() __asm__ __volatile__("dmb ish" ::: "memory")
#define mint_thread_fence_seq_cst() __asm__ __volatile__("dmb ish" ::: "memory")


//----------------------------------------------
//  32-bit atomic operations
//----------------------------------------------
MINT_C_INLINE uint32_t mint_load_32_relaxed(const mint_atomic32_t *object)
{
    return object->_nonatomic;
}

MINT_C_INLINE void mint_store_32_relaxed(mint_atomic32_t *object, uint32_t desired)
{
    object->_nonatomic = desired;
}

MINT_C_INLINE uint32_t mint_compare_exchange_strong_32_relaxed(mint_atomic32_t *object, uint32_t expected, uint32_t desired)
{
    uint32_t status;
    uint32_t original;
    __asm__ __volatile__("1:     ldxr    %w0, [%3]\n"
                         "       cmp     %w0, %w4\n"
                         "       b.ne    2f\n"
                         "       stxr    %w1, %w5, [%3]\n"
                         "       cbnz    %w1, 1b\n"
                         "2:     ;"
                         : "=&r"(original), "=&r"(status), "+Q"(object->_nonatomic)
                         : "r"(object), "r"(expected), "r"(desired)
                         : "cc", "memory");
    return original;
}

MINT_C_INLINE uint32_t mint_fetch_add_32_relaxed(mint_atomic32_t *object, int32_t operand)
{
    uint32_t status;
    uint32_t original, desired;
    __asm__ __volatile__("1:     ldxr    %w0, [%4]\n"
                         "       add     %w3, %w0, %w5\n"
                         "       stxr    %w1, %w3, [%4]\n"
                         "       cbnz    %w1, 1b"
                         : "=&r"(original), "=&r"(status), "+Q"(object->_nonatomic), "=&r"(desired)
                         : "r"(object), "r"(operand)
                         : "cc", "memory");
    return original;
}

MINT_C_INLINE uint32_t mint_fetch_and_32_relaxed(mint_atomic32_t *object, uint32_t operand)
{
    uint32_t status;
    uint32_t original, desired;
    __asm__ __volatile__("1:     ldxr    %w0, [%4]\n"
                         "       and     %w3, %w0, %w5\n"
                         "       stxr    %w1, %w3, [%4]\n"
                         "       cbnz    %w1, 1b"
                         : "=&r"(original), "=&r"(status), "+Q"(object->_nonatomic), "=&r"(desired)
                         : "r"(object), "r"(operand)
                         : "cc", "memory");
    return original;
}

MINT_C_INLINE uint32_t mint_fetch_or_32_relaxed(mint_atomic32_t *object, uint32_t operand)
{
    uint32_t status;
    uint32_t original, desired;
    __asm__ __volatile__("1:     ldxr    %w0, [%4]\n"
                         "       orr     %w3, %w0, %w5\n"
                         "       stxr    %w1, %w3, [%4]\n"
                         "       cbnz    %w1, 1b"
                         : "=&r"(original), "=&r"(status), "+Q"(object->_nonatomic), "=&r"(desired)
                         : "r"(object), "r"(operand)
                         : "cc", "memory");
    return original;
}


//----------------------------------------------
//  64-bit atomic operations
//----------------------------------------------
MINT_C_INLINE uint64_t mint_load_64_relaxed(const mint_atomic64_t *object)
{
    return object->_nonatomic;
}

MINT_C_INLINE void mint_store_64_relaxed(mint_atomic64_t *object, uint64_t desired)
{
    object->_nonatomic = desired;
}

MINT_C_INLINE uint64_t mint_compare_exchange_strong_64_relaxed(mint_atomic64_t *object, uint64_t expected, uint64_t desired)
{
    uint32_t status;
    uint64_t original;
    __asm__ __volatile__("1:     ldxr    %0, [%3]\n"
                         "       cmp     %0, %4\n"
                         "       b.ne    2f\n"
                         "       stxr    %w1, %5, [%3]\n"
                         "       cbnz    %w1, 1b\n"
                         "2:     ;"
                         : "=&r"(original), "=&r"(status), "+Q"(object->_nonatomic)
                         : "r"(object), "r"(expected), "r"(desired)
                         : "cc", "memory");
    return original;
}

MINT_C_INLINE uint64_t mint_fetch_add_64_relaxed(mint_atomic64_t *object, int64_t operand)
{
    uint32_t status;
    uint64_t original, desired;
    __asm__ __volatile__("1:     ldxr    %0, [%4]\n"
                         "       add     %3, %0, %5\n"
                         "       stxr    %w1, %3, [%4]\n"
                         "       cbnz    %w1, 1b"
                         : "=&r"(original), "=&r"(status), "+Q"(object->_nonatomic), "=&r"(desired)
                         : "r"(object), "r"(operand)
                         : "cc", "memory");
    return original;
}

MINT_C_INLINE uint64_t mint_fetch_and_64_relaxed(mint_atomic64_t *object, uint64_t operand)
{
    uint32_t status;
    uint64_t original, desired;
    __asm__ __volatile__("1:     ldxr    %0, [%4]\n"
                         "       and     %3, %0, %5\n"
                         "       stxr    %w1, %3, [%4]\n"
                         "       cbnz    %w1, 1b"
                         : "=&r"(original), "=&r"(status), "+Q"(object->_nonatomic), "=&r"(desired)
                         : "r"(object), "r"(operand)
                         : "cc", "memory");
    return original;
}

MINT_C_INLINE uint64_t mint_fetch_or_64_relaxed(mint_atomic64_t *object, uint64_t operand)
{
    uint32_t status;
    uint64_t original, desired;
    __asm__ __volatile__("1:     ldxr    %0, [%4]\n"
                         "       orr     %3, %0, %5\n"
                         "       stxr    %w1, %3, [%4]\n"
                         "       cbnz    %w1, 1b"
                         : "=&r"(original), "=&r"(status), "+Q"(object->_nonatomic), "=&r"(desired)
                         : "r"(object), "r"(operand)
                         : "cc", "memory");
    return original;
}


#ifdef __cplusplus
} // extern "C"
#endif

#endif // __MINTOMIC_PRIVATE_MINTOMIC_GCC_ARM64_H__
