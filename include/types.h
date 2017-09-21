#ifndef _LINUX_TYPES_H_
#define _LINUX_TYPES_H_
#include <sys/types.h>

#ifndef NULL
#define NULL (void *)0
#endif

typedef char  s8;
typedef short s16;
typedef long  s32;
typedef long long s64;

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;
typedef unsigned long long u64;

typedef s32 status_t;

struct list_head {
	struct list_head *next, *prev;
};

//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr); \
	(type *) ( (char *)__mptr - offsetof(type, member) );})

#endif /* _LINUX_TYPES_H_ */
