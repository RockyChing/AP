#ifndef _LINUX_LOG_H
#define _LINUX_LOG_H
#include <stdio.h>


#define LOG_EMERG	0	/* system is unusable */
#define LOG_ALERT	1	/* action must be taken immediately */
#define LOG_CRIT	2	/* critical conditions */
#define LOG_ERR	3	/* error conditions */
#define LOG_WARNING	4	/* warning conditions */
#define LOG_NOTICE	5	/* normal but significant condition */
#define LOG_INFO	6	/* informational */
#define LOG_DEBUG	7	/* debug-level messages */

#define LOG_VALID   LOG_DEBUG
const int logtofile = 0;

#define LOG(fmt, ...) do {                                                               \
    if (0 <= LOG_VALID) {                                                                \
        if (logtofile) {                                                                 \
            fprintf(NULL, "%s(%d): " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
            fflush(NULL);                                                                \
        } else {                                                                         \
            fprintf(stderr, "%s(%d): " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        }                                                                                \
    }                                                                                    \
} while (0)

#endif

