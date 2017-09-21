#ifndef _LINUX_CONDITION_T
#define _LINUX_CONDITION_T

#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#include <time.h>
#include <types.h>

class Condition {
public:
    enum {
        PRIVATE = 0,
        SHARED = 1
    };

    enum WakeUpType {
        WAKE_UP_ONE = 0,
        WAKE_UP_ALL = 1
    };

    Condition();
    Condition(int type);
    ~Condition();

    // wait on the condition variable, lock the mutex before calling
    status_t wait(Mutex& mutex);
    // same with relative timeout
    status_t waitRelative(Mutex& mutex, nsecs_t reltime);
    // signal the condition variable, allowing exactly one thread to continue
    void signal();
    // signal the condition variable, allowing one or all threads to continue
    void signal(WakeUpType type) {
        if (type == WAKE_UP_ONE)
            signal();
        else
            broadcast();
    }

    // signal the condition variable, allowing all threads to continue
    void broadcast();

private:
    pthread_cond_t mCond;
};

inline Condition::Condition() {
    pthread_cond_init(&mCond, NULL);
}

inline Condition::Condition(int type) {
    if (SHARED == type) {
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);
        pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&mCond, &attr);
        pthread_condattr_destroy(&attr);
    } else {
        pthread_cond_init(&mCond, NULL);
    }
}

inline Condition::~Condition() {
    pthread_cond_destroy(&mCond);
}

inline status_t Condition::wait(Mutex& mutex) {
    return -pthread_cond_wait(&mCond, &mutex.mMutex);
}

inline status_t Condition::waitRelative(Mutex& mutex, nsecs_t reltime) {
#if defined(HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE)
    struct timespec ts;
    ts.tv_sec  = reltime/1000000000;
    ts.tv_nsec = reltime%1000000000;
    return -pthread_cond_timedwait_relative_np(&mCond, &mutex.mMutex, &ts);
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += reltime/1000000000;
    ts.tv_nsec += reltime%1000000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec  += 1;
    }

    return -pthread_cond_timedwait(&mCond, &mutex.mMutex, &ts);
#endif
}

inline void Condition::signal() {
    /*
     * POSIX says pthread_cond_signal wakes up "one or more" waiting threads.
     * However bionic follows the glibc guarantee which wakes up "exactly one"
     * waiting thread.
     *
     * man 3 pthread_cond_signal
     *   pthread_cond_signal restarts one of the threads that are waiting on
     *   the condition variable cond. If no threads are waiting on cond,
     *   nothing happens. If several threads are waiting on cond, exactly one
     *   is restarted, but it is not specified which.
     */
    pthread_cond_signal(&mCond);
}

inline void Condition::broadcast() {
    pthread_cond_broadcast(&mCond);
}

#endif

