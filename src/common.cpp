#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>


#include <types.h>


static u8 month_day_table[12] = {
    0  , 31 , 28 , 31 ,
    30 , 31 , 30 , 31 , 
    31 , 30 , 31 , 30
};

u8 get_day_of_month(u8 month)
{
    return month >= 12 ? 0 : month_day_table[month];
}

static bool isLeapYear(u32 year)
{
    bool ret = false;
    do {
        if ((year % 4) != 0x00000000)
            break;

        if ((year % 100) != 0x00000000) {
            ret = true;
            break;
        }

        if ((year % 400) != 0x00000000)
            break;

        if ((year % 3200) == 0x00000000)
            break;
        ret = true;
    } while (0);

    return ret;
}

u32 get_tick()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec/(1000000);

}

int str_split(const char *in, int len, /* input & input len */
    const char sparator, /* the sparator */
    str_split_result_t *out /* result  */)
{
    int ret = 0;
    int i = 0;
    int index = 0; // index of array
    int shift = 0; // index of sub-string array

    if (in == NULL || out == NULL) {
        out->err = STR_ERR_NULL;
        return -1;
    }

    if (len <= 0 || len > STR_SPLIT_CNT * STR_SPLIT_SUB_LEN) {
        out->err = STR_ERR_LEN;
        return -1;
    }

    while (i < len) {
        if (sparator != ' ' && isspace(in[i]))
            continue;

        if (in[i] == sparator) {
            out->sub[index][shift] = '\0';
            shift = 0;
            index ++;
            if (index >= STR_SPLIT_CNT) { // out index of array
                out->err = STR_ERR_OUT_INDEX;
                ret = -1;
                break;
            }
        } else {
            if (shift >= STR_SPLIT_SUB_LEN) { // out index of string array
                out->sub[index][STR_SPLIT_SUB_LEN - 1] = '\0';
                out->err = STR_ERR_OUT_INDEX;
                ret = -1;
                break;
            } else {
                out->sub[index][shift++] = in[i];
            }
        }
        i ++;
    }

    if (index != 0) {
        out->count = index + 1;
    } else {
        out->err = STR_ERR_IGNOR;
        ret = -1;
    }
    return ret;
}

