#include <stdio.h>


#include <types.h>
#include <log.h>
#include <atomic.h>
#include <Mutex.h>
#include <Condition.h>
#include <StrongPointer.h>

Mutex mLock;
Condition mCond;

static void check_builtin_type_size()
{
    AutoMutex l(mLock);
    LOG("sizeof(char): %d", sizeof(char));
    LOG("sizeof(short): %d", sizeof(short));
    LOG("sizeof(long): %d", sizeof(long));
    LOG("sizeof(long long): %d", sizeof(long long));
}













int main(int argc, char **argv)
{
    (void)argc;
    int atomic_int = 1;
    int ret;
    
    printf("%s enter.\n", argv[0]);
    check_builtin_type_size();
    //mCond.wait(mLock);
    ret = android_atomic_inc(&atomic_int);
    LOG("atomic_int: %d", atomic_int);
    LOG("ret: %d", ret);

    sleep(4);
    printf("%s exit.\n", argv[0]);
    return 0;
}

