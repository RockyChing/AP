#ifndef _LINUX_THREAD_DEF_H
#define _LINUX_THREAD_DEF_H

#ifdef __cplusplus
extern "C" {
#endif
typedef int (*thread_entry_func_t)(void*);
#ifdef __cplusplus
} // extern "C"
#endif

typedef void* thread_id_t;
typedef thread_entry_func_t thread_func_t;

enum {
    PRIORITY_DEFAULT = 0,
};

#endif

