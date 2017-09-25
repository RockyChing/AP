#ifndef _LINUX_COMMON_H
#define _LINUX_COMMON_H
#include <stdio.h>
#include <unistd.h>
#include <string.h>


#include <types.h>
#include <log.h>

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) :(b))
#endif

#define ASSERT(v) do {                                                 \
    if ((v) < 0) {                                                     \
        LOG("system-error: '%s' (code: %d)", strerror(errno), errno);  \
        return -1; }                                                   \
    } while (0)

#define NUM_ELEMENTS(arr) (sizeof(arr) / sizeof(arr[0]))


void inline sleep_s(int32_t sec)
{
    usleep(sec * 1000 * 1000);
}

void inline sleep_ms(int32_t msec)
{
    usleep(msec * 1000);
}

/**
 * Thread safe & MT-Safe
 */ 
void inline sleep_us(useconds_t usec)
{
    usleep(usec);
}

u8  get_day_of_month(u8 month);
u32 get_tick();
int str_split(const char *in, int len, /* input & input len */
    char sparator, /* the sparator */
    str_split_result_t *out /* result  */);






#endif
