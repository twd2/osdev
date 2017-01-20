#include "asm.h"

#include <runtime/types.h>

inline bool test_and_set(bool *x)
{
    bool ret;
    asm volatile ("lock bts $0, %1\n"
                  "adc $0, %0" // use setc instead?
                  : "=g"(ret)
                  : "m"(*x), "0"(0));
    return ret;
}

inline bool test_and_reset(bool *x)
{
    bool ret;
    asm volatile ("lock btr $0, %1\n"
                  "adc $0, %0"
                  : "=g"(ret)
                  : "m"(*x), "0"(0));
    return ret;
}

// returns old (*x)
inline ureg_t compare_and_swap(ureg_t *x, ureg_t compare_value, ureg_t swap_value)
{
    ureg_t ret;
    asm volatile ("lock cmpxchg %3, %1"
                  : "=a"(ret)
                  : "m"(*x), "a"(compare_value), "r"(swap_value));
    return ret;
}

inline void spinlock_init(spinlock_t *l)
{
    l->lock = false;
}

inline bool spinlock_try_lock(spinlock_t *l)
{
    return !test_and_set(&l->lock); // if old value is 0, we get the lock
}

void spinlock_wait_and_lock(spinlock_t *l)
{
    while (!spinlock_try_lock(l)); // loop until we get the lock
}

void spinlock_release(spinlock_t *l)
{
    l->lock = false;
}
