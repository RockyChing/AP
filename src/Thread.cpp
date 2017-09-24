#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(__linux__)
#include <sys/prctl.h>
#endif

#include <sched_policy.h>
#include <Mutex.h>
#include <Thread.h>
#include <Errors.h>
#include <log.h>
#define unused __attribute__((__unused__))

typedef void* (*pthread_entry)(void*);

#ifdef HAVE_ANDROID_OS
static pthread_t android_thread_id_t_to_pthread(thread_id_t thread)
{
    return (pthread_t) thread;
}
#endif

// Get some sort of unique identifier for the current thread.
inline thread_id_t getThreadId() {
    return (thread_id_t) pthread_self();
}

static int createRawThread(thread_entry_func_t entryFunction,
        void *userData, const char *threadName,
        int32_t threadPriority,
        size_t threadStackSize,
        thread_id_t *threadId)
{
    (void) threadPriority; // not support yet, avoid warning
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (threadStackSize) {
        pthread_attr_setstacksize(&attr, threadStackSize);
    }

    errno = 0;
    pthread_t thread;
    int result = pthread_create(&thread, &attr,
                    (pthread_entry) entryFunction, userData);
    pthread_attr_destroy(&attr);
    if (result != 0) {
        LOG("createRawThread failed (entry=%p, res=%d, errno=%d)\n"
             "(android threadPriority=%d)",
            entryFunction, result, errno, threadPriority);
        return 0;
    } else {
        LOG("createRawThread \"%s\" success.", threadName ? threadName : "default");
    }

    // Note that *threadID is directly available to the parent only, as it is
    // assigned after the child starts.  Use memory barrier / lock if the child
    // or other threads also need access.
    if (threadId != NULL) {
        *threadId = (thread_id_t) thread; // XXX: this is not portable
    }
    return 1;
}
//-------------------------------------------------------------------------------------


/*
 * This is our thread object!
 */
 Thread::Thread():
    mThread(thread_id_t(-1)),
    mStatus(NO_ERROR),
    mExitPending(false),
    mRunning(false),
    mTid(-1)
{ }

Thread::~Thread()
{ }

status_t Thread::readyToRun()
{
    return NO_ERROR;
}

status_t Thread::run(const char * name, int32_t priority, size_t stack)
{
    Mutex::AutoLock _l(mLock);

    if (mRunning) {
        // Thread already started
        return INVALID_OPERATION;
    }

    /**
     * reset status and exitPending to their default value, so we can
     * try again after an error happened (either below, or in readyToRun())
     */
    mStatus = NO_ERROR;
    mExitPending = false;
    mThread = thread_id_t(-1);

    // hold a strong reference on ourself
    mHoldSelf = this;

    mRunning = true;

    bool res = createRawThread(_threadLoop, this, name, priority, stack, &mThread);
    if (false == res) {
        mStatus = UNKNOWN_ERROR;
        mRunning = false;
        mThread = thread_id_t(-1);
        mHoldSelf.clear(); // "this" may have gone away after this.
        return UNKNOWN_ERROR;
    }

    /**
     * Do not refer to mStatus here: The thread is already running (may, in fact
     * already have exited with a valid mStatus result). The NO_ERROR indication
     * here merely indicates successfully starting the thread and does not
     * imply successful termination/execution.
     */
    return NO_ERROR;
    // Exiting scope of mLock is a memory barrier and allows new thread to run
}

int Thread::_threadLoop(void* user)
{
    Thread* const self = static_cast<Thread*>(user);

    sp<Thread> strong(self->mHoldSelf);
    wp<Thread> weak(strong);
    self->mHoldSelf.clear();

    // this is very useful for debugging with gdb
    self->mTid = gettid();

    bool first = true;
    do {
        bool result;
        if (first) {
            first = false;
            self->mStatus = self->readyToRun();
            result = (self->mStatus == NO_ERROR);
            if (result && !self->exitPending()) {
                // Binder threads (and maybe others) rely on threadLoop
                // running at least once after a successful ::readyToRun()
                // (unless, of course, the thread has already been asked to exit
                // at that point).
                // This is because threads are essentially used like this:
                //   (new ThreadSubclass())->run();
                // The caller therefore does not retain a strong reference to
                // the thread and the thread would simply disappear after the
                // successful ::readyToRun() call instead of entering the
                // threadLoop at least once.
                result = self->threadLoop();
            }
        } else {
            result = self->threadLoop();
        }

        // establish a scope for mLock
        {
            Mutex::AutoLock _l(self->mLock);
            if (result == false || self->mExitPending) {
                self->mExitPending = true;
                self->mRunning = false;
                // clear thread ID so that requestExitAndWait() does not exit if
                // called by a new thread using the same thread ID as this one.
                self->mThread = thread_id_t(-1);
                // note that interested observers blocked in requestExitAndWait are
                // awoken by broadcast, but blocked on mLock until break exits scope
                self->mThreadExitedCondition.broadcast();
                break;
            }
        }

        // Release our strong reference, to let a chance to the thread
        // to die a peaceful death.
        strong.clear();
        // And immediately, re-acquire a strong reference for the next loop
        strong = weak.promote();
    } while(strong != 0);

    return 0;
}

void Thread::requestExit()
{
    Mutex::AutoLock _l(mLock);
    mExitPending = true;
}

status_t Thread::requestExitAndWait()
{
    Mutex::AutoLock _l(mLock);
    if (mThread == getThreadId()) {
        LOG(
        "Thread (this=%p): don't call waitForExit() from this "
        "Thread object's thread. It's a guaranteed deadlock!",
        this);

        return WOULD_BLOCK;
    }

    mExitPending = true;

    while (mRunning == true) {
        mThreadExitedCondition.wait(mLock);
    }
    // This next line is probably not needed any more, but is being left for
    // historical reference. Note that each interested party will clear flag.
    mExitPending = false;

    return mStatus;
}

status_t Thread::join()
{
    Mutex::AutoLock _l(mLock);
    if (mThread == getThreadId()) {
        LOG(
        "Thread (this=%p): don't call join() from this "
        "Thread object's thread. It's a guaranteed deadlock!",
        this);

        return WOULD_BLOCK;
    }

    while (mRunning == true) {
        mThreadExitedCondition.wait(mLock);
    }

    return mStatus;
}

bool Thread::isRunning() const {
    Mutex::AutoLock _l(mLock);
    return mRunning;
}

pid_t Thread::getTid() const
{
    // mTid is not defined until the child initializes it, and the caller may need it earlier
    Mutex::AutoLock _l(mLock);
    pid_t tid;
    if (mRunning) {
        pthread_t pthread = (pthread_t) mThread;
        tid = pthread_gettid_np(pthread);
    } else {
        LOG("Thread (this=%p): getTid() is undefined before run()", this);
        tid = -1;
    }

    return tid;
}

bool Thread::exitPending() const
{
    Mutex::AutoLock _l(mLock);
    return mExitPending;
}


