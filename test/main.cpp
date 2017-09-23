#include <stdio.h>


#include <types.h>
#include <log.h>
#include <atomic.h>
#include <Mutex.h>
#include <Condition.h>
#include <StrongPointer.h>


#include "RefBaseTest.h"
#include "ThreadTest.h"

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

static void refbase_test()
{
#if 1
    sp<RefBaseTest> pRefBaseTest = new RefBaseTest(1);
    LOG("data before set: %d", pRefBaseTest->getData());
    pRefBaseTest->setData(5);
    LOG("data after set: %d", pRefBaseTest->getData());
#else
    RefBaseTest *pRefBaseTest = new RefBaseTest(2);
    LOG("data before set: %d", pRefBaseTest->getData());
    pRefBaseTest->setData(51);
    LOG("data after set: %d", pRefBaseTest->getData());
    delete pRefBaseTest;
#endif
}

static void thread_test()
{
    sp<ThreadTest> pThreadTest = new ThreadTest();
    pThreadTest->run();
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

    refbase_test();
    thread_test();

    while (1) {
        sleep(4);
    }

    printf("%s exit.\n", argv[0]);
    return 0;
}

