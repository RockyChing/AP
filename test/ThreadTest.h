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
    void onFirstRef();
    void onLastStrongRef(const void *);
    void onLastWeakRef(const void *);

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
    int count = 0;
    while (count++ < 5) {
        LOG("ThreadTest::threadLoop");
        sleep(1);
    }

    return true;
}

void ThreadTest::onFirstRef()
{
    LOG("ThreadTest onFirstRef");
}

void ThreadTest::onLastStrongRef(const void *)
{
    LOG("ThreadTest onLastStrongRef");
}

void ThreadTest::onLastWeakRef(const void *)
{
    LOG("ThreadTest onLastWeakRef");
}


#endif

