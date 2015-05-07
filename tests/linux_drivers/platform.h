#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "langinc.h"
#include "types.h"

typedef u64 phys_addr_t;
typedef phys_addr_t resource_size_t;

conditional_t cond_irq_enabled;
lock_t irq_running_lock;

void request_irq () {
    notify(cond_irq_enabled);
}

void free_irq() {
    reset(cond_irq_enabled);
    lock(irq_running_lock);
    unlock(irq_running_lock);
}

struct resource {
        resource_size_t start;
        resource_size_t end;
        const char *name;
        unsigned long flags;
        //struct resource *parent, *sibling, *child;
};

static inline resource_size_t resource_size(const struct resource *res)
{
        return (resource_size_t) nondet;
}

conditional_t cond_platform_driver_registered;

int platform_driver_register() {
    if (nondet) {
        notify (cond_platform_driver_registered);
        return 0;
    } else {
        return -1;
    }
}

void platform_driver_unregister() {
    reset (cond_platform_driver_registered);
}

int platform_get_irq_byname(const char* name) {
    ioval = nondet;
    return (int)ioval;
}

struct resource * platform_get_resource_byname(unsigned int type, const char * name) {
    return (struct resource *)(addr_t)nondet;
}

struct resource * request_mem_region(resource_size_t start, resource_size_t n, const char *name) {
    return (struct resource *)(addr_t)nondet;
}

void release_mem_region(resource_size_t start, resource_size_t n)
{
    ioval = nondet;
}

#endif
