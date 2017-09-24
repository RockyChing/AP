#include <stdio.h>


#include <types.h>
#include <log.h>
#include <atomic.h>
#include <Mutex.h>
#include <Condition.h>
#include <StrongPointer.h>
#include <Errors.h>

#include "RefBaseTest.h"
#include "ThreadTest.h"

#define TEST_SIZE      0
#define TEST_REFBASE   0
#define TEST_THREAD    1
#define TEST_STR_SPLIT 1





Mutex mLock;
Condition mCond;

#if TEST_SIZE
static void check_builtin_type_size()
{
    AutoMutex l(mLock);
    LOG("sizeof(char): %d", sizeof(char));
    LOG("sizeof(short): %d", sizeof(short));
    LOG("sizeof(long): %d", sizeof(long));
    LOG("sizeof(long long): %d", sizeof(long long));
}
#endif

#if TEST_REFBASE
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
#endif

#if TEST_THREAD
static void thread_test()
{
    sp<ThreadTest> pThreadTest = new ThreadTest();
    if (pThreadTest->readyToRun() == NO_ERROR) {
        LOG("run thread");
        pThreadTest->run("ThreadTest", PRIORITY_DEFAULT, 0);
    }
}
#endif

#if TEST_STR_SPLIT
static void str_split_test(const char *data, const char sepatator)
{
    int i;
    str_split_result_t result;
    memset(&result, 0, sizeof(result));

    if (str_split(data, strlen(data), sepatator, &result)) {
        LOG("error: %d", result.err);
    } else {
        for (i = 0; i < result.count; i ++)
            LOG("%s", result.sub[i]);
    }

    LOG("\n");
}
#endif






int main(int argc, char **argv)
{
    (void)argc;
    int atomic_int = 1;
    int ret;
    
    printf("%s enter.\n", argv[0]);
#if TEST_SIZE
    check_builtin_type_size();
    //mCond.wait(mLock);
#endif

#if TEST_REFBASE
    refbase_test();
#endif

#if TEST_THREAD
    thread_test();
#endif

#if TEST_STR_SPLIT
    const char *data1 = "pointed to by the argument tp";
    str_split_test(data1, ' ');

    const char *data2 = "android.permission.INTERNET.12.android.permission.INTERNET,android.permission.INTERNET,android.permission.INTERNET,android.permission.INTERNET,android.permission.INTERNET,android.permission.INTERNET,android.permission.INTERNET,android.permission.INTERNET,android.permission.INTERNET,android.permission.INTERNET";
    str_split_test(data2, '.');

    const char *data3 = "$GPRMC,024813.640,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*50";
    str_split_test(data3, ',');
#endif

    while (1) {
        sleep(4);
    }

    printf("%s exit.\n", argv[0]);
    return 0;
}

