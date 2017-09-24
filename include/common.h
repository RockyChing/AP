#ifndef _LINUX_COMMON_H
#define _LINUX_COMMON_H
#include <unistd.h>

#include <types.h>

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
