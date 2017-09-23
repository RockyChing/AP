#ifndef _LINUX_THREAD_TEST_H
#define _LINUX_THREAD_TEST_H
#include <Thread.h>
#include <log.h>
#include <time.h>


class ThreadTest: public Thread {
public:
     ThreadTest();
    ~ThreadTest();

private:
    bool threadLoop();

private:
    int mCount;
};

ThreadTest::ThreadTest():
    mCount(1)
{
    LOG("ThreadTest create with mCount = %d", mCount);
}

ThreadTest::~ThreadTest()
{
    LOG("ThreadTest destroy with mCount = %d", mCount);
}

bool ThreadTest::threadLoop()
{
    while (1) {
        LOG("ThreadTest::threadLoop");
        sleep(1);
    }

    return true;
}


#endif

