#ifndef _TYPES_H_
#define _TYPES_H_

#include "langinc.h"

conditional_t can_sleep;


typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

typedef u32 __le32;
typedef u64 __le64;

typedef unsigned int       size_t;

typedef int bool;
const bool true  = 1;
const bool false = 0;

typedef int atomic_t;

enum irqreturn {
        IRQ_NONE                = (0 << 0),
        IRQ_HANDLED             = (1 << 0),
        IRQ_WAKE_THREAD         = (1 << 1),
};

typedef enum irqreturn irqreturn_t;

#define USHRT_MAX       ((u16)(~0U))
#define SHRT_MAX        ((s16)(USHRT_MAX>>1))
#define SHRT_MIN        ((s16)(-SHRT_MAX - 1))
#define INT_MAX         ((int)(~0U>>1))
#define INT_MIN         (-INT_MAX - 1)
#define UINT_MAX        (~0U)
#define LONG_MAX        ((long)(~0UL>>1))
#define LONG_MIN        (-LONG_MAX - 1)
#define ULONG_MAX       (~0UL)
#define LLONG_MAX       ((long long)(~0ULL>>1))
#define LLONG_MIN       (-LLONG_MAX - 1)
#define ULLONG_MAX      (~0ULL)
#define SIZE_MAX        (~(size_t)0)

#define U8_MAX          ((u8)~0U)
#define S8_MAX          ((s8)(U8_MAX>>1))
#define S8_MIN          ((s8)(-S8_MAX - 1))
#define U16_MAX         ((u16)~0U)
#define S16_MAX         ((s16)(U16_MAX>>1))
#define S16_MIN         ((s16)(-S16_MAX - 1))
#define U32_MAX         ((u32)~0U)
#define S32_MAX         ((s32)(U32_MAX>>1))
#define S32_MIN         ((s32)(-S32_MAX - 1))
#define U64_MAX         ((u64)~0ULL)
#define S64_MAX         ((s64)(U64_MAX>>1))
#define S64_MIN         ((s64)(-S64_MAX - 1))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define max(x,y) ({ \
        typeof(x) _x = (x);     \
        typeof(y) _y = (y);     \
        _x > _y ? _x : _y; })

#define NULL ((void *)0)

#define unlikely(x) x
#define likely(x) x

#define cpu_to_le64(x) x
#define cpu_to_le32(x) x
#define le32_to_cpu(x) x
#define le64_to_cpu(x) x

#define spin_lock(_l) lock(_l)
#define spin_unlock(_l) unlock(_l)

u16 swab16(u16 x)
{
    return x;
}

u32 swab32(u32 x)
{
    return x;
}

//# define __iomem        __attribute__((noderef, address_space(2)))

#define __iomem

typedef u64 dma_addr_t;

#define set_bit(nr,addr) {addr[0]=nr;}
#define clear_bit(nr,addr) {addr[0]=nr;}

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, 8 * sizeof(long))
#define DECLARE_BITMAP(name,bits) \
        unsigned long name[BITS_TO_LONGS(bits)]


#define strncpy(_to, _frm, _n) ({(_to)[_n-_n]=(_frm)[_n-_n]; _to;})
#define memcpy(_to, _frm, _n) ({((char*)(_to))[_n-1]=((char*)(_frm))[_n-1]; _to;})
#define snprintf(_buf, _size, _fmt ...) {(_buf)[_size-_size]=(_fmt)[_size-_size];}

struct work_struct {
    int _opaque;
};

void cancel_work_sync()
{
    // TODO
};

#define mutex_lock(l) lock(l)

#define mutex_unlock(l) unlock(l)

u64 ioval;

#define assert(c) ioval = (u64)c

#define BUG_ON(c)        assert(!(c))
#define BUG()            assert(false)
#define WARN_ON_ONCE(c)  assert(!(c))
#define mmiowb()

void msleep (unsigned int msecs) {}
void udelay (unsigned long long usecs) {}

#define IORESOURCE_TYPE_BITS    0x00001f00      /* Resource type */
#define IORESOURCE_IO           0x00000100      /* PCI/ISA I/O ports */
#define IORESOURCE_MEM          0x00000200
#define IORESOURCE_REG          0x00000300      /* Register offsets */
#define IORESOURCE_IRQ          0x00000400
#define IORESOURCE_DMA          0x00000800
#define IORESOURCE_BUS          0x00001000

#define EPERM            1      /* Operation not permitted */
#define ENOENT           2      /* No such file or directory */
#define ESRCH            3      /* No such process */
#define EINTR            4      /* Interrupted system call */
#define EIO              5      /* I/O error */
#define ENXIO            6      /* No such device or address */
#define E2BIG            7      /* Argument list too long */
#define ENOEXEC          8      /* Exec format error */
#define EBADF            9      /* Bad file number */
#define ECHILD          10      /* No child processes */
#define EAGAIN          11      /* Try again */
#define ENOMEM          12      /* Out of memory */
#define EACCES          13      /* Permission denied */
#define EFAULT          14      /* Bad address */
#define ENOTBLK         15      /* Block device required */
#define EBUSY           16      /* Device or resource busy */
#define EEXIST          17      /* File exists */
#define EXDEV           18      /* Cross-device link */
#define ENODEV          19      /* No such device */
#define ENOTDIR         20      /* Not a directory */
#define EISDIR          21      /* Is a directory */
#define EINVAL          22      /* Invalid argument */
#define ENFILE          23      /* File table overflow */
#define EMFILE          24      /* Too many open files */
#define ENOTTY          25      /* Not a typewriter */
#define ETXTBSY         26      /* Text file busy */
#define EFBIG           27      /* File too large */
#define ENOSPC          28      /* No space left on device */
#define ESPIPE          29      /* Illegal seek */
#define EROFS           30      /* Read-only file system */
#define EMLINK          31      /* Too many links */
#define EPIPE           32      /* Broken pipe */
#define EDOM            33      /* Math argument out of domain of func */
#define ERANGE          34      /* Math result not representable */

#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))

#define atomic_set(_var, _val) {_var = _val;}
#define atomic_read(_var) (_var)
#define atomic_inc(_var) {_var=_var+1;}

typedef u64 phys_addr_t;
typedef u64 addr_t;
typedef phys_addr_t resource_size_t;

void __iomem *ioremap(resource_size_t offset, unsigned long size)
{
    ioval = offset + size;
    return (void *) ioval;
}

#define iounmap(addr) {*(int*)addr=nondet; ioval=nondet;}

//typedef struct seqcount {
//    unsigned sequence;
//} seqcount_t;

struct u64_stats_sync {
};

#define u64_stats_init(sync)

#define HZ 100


int device_set_wakeup_enable(bool enable)
{
    ioval = enable;
    return ioval;
};

#define GFP_KERNEL     0x1 
typedef unsigned gfp_t;

enum dma_data_direction {
         DMA_BIDIRECTIONAL = 0,
         DMA_TO_DEVICE = 1,
         DMA_FROM_DEVICE = 2,
         DMA_NONE = 3,
};

//void * dma_alloc_coherent(size_t size, dma_addr_t *dma_handle, gfp_t gfp)
#define dma_alloc_coherent(size,dma_handle,gfp) ({ \
    dma_addr_t x;               \
    void* y;                    \
    *(dma_handle) = x;            \
    y;                          \
})

#define dma_free_coherent(s,c,h) { \
    c = NULL;\
    h = 0;\
}

//dma_addr_t dma_map_single(void *ptr, size_t size,
//                          enum dma_data_direction direction)
#define dma_map_single(ptr, size, direction) ({\
        dma_addr_t x; \
        x;            \
})

//void dma_unmap_single(dma_addr_t handle, size_t size, enum dma_data_direction dir)
void dma_unmap_single(dma_addr_t handle, size_t size, enum dma_data_direction dir)
{
    // TODO: model potential race with anyone using the handle
};

int dma_mapping_error(dma_addr_t dma_addr)
{
        int x;
        return x;
}

#define memset(x,v,s) {*(int*)(x) = 0;}

void *kmalloc_node(size_t size, gfp_t flags, int node)
{
    return (void*) (u64)nondet;

}

#define kfree(p) {*(int*)(p)=0;}

//void kfree(void * p)
//{
//    // model the fact that the value of p changes
//    *(int*)p = 0;
//}

#define ALIGN(x, a)                  __ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_KERNEL_MASK(x, mask) (((x) + (mask)) & ~(mask))

#define dma_wmb()
#define smp_mb()

unsigned long volatile jiffies;



#endif // _TYPES_H_
