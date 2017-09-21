#ifndef _LINUX_MUTEX_H
#define _LINUX_MUTEX_H

#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#include <time.h>
#include <types.h>


class Condition;

/*
 * Simple mutex class.  The implementation is system-dependent.
 *
 * The mutex must be unlocked by the thread that locked it.  They are not
 * recursive, i.e. the same thread can't lock it multiple times.
 */
class Mutex {
public:
    enum {
        PRIVATE = 0,
        SHARED = 1
    };

    Mutex();
    Mutex(int type);
    ~Mutex();

    status_t lock();
    void     unlock();

    // lock if possible; return 0 on sucess, error otherwise
    status_t tryLock();

    /**
     * Manages the mutex automatically. It'll be locked when AutoLock is 
     * constructed and relased when AutoLock goes out of scope
     */
    class AutoLock {
    public:
        inline AutoLock(Mutex& mutex) : mLock(mutex) { mLock.lock(); }
        inline AutoLock(Mutex *mutex) : mLock(*mutex) { mLock.lock(); }
        inline ~AutoLock() { mLock.unlock(); }
    private:
        Mutex &mLock;
    };

private:
    friend class Condition;
    // Mutex must not be copied!
           Mutex(const Mutex &);
    Mutex& operator=(const Mutex &);

    pthread_mutex_t mMutex;
};

inline Mutex::Mutex() {
    pthread_mutex_init(&mMutex, NULL);
}

inline Mutex::Mutex(int type) {
    if (SHARED == type) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mMutex, &attr);
        pthread_mutexattr_destroy(&attr);
    } else {
        pthread_mutex_init(&mMutex, NULL);
    }
}

inline Mutex::~Mutex() {
    pthread_mutex_destroy(&mMutex);
}

inline status_t Mutex::lock() {
    printf("lock\n");
    return -pthread_mutex_lock(&mMutex);
}

inline void Mutex::unlock() {
    printf("unlock\n");
    pthread_mutex_unlock(&mMutex);
}

inline status_t Mutex::tryLock() {
    return -pthread_mutex_trylock(&mMutex);
}

/*
 * Automatic mutex.  Declare one of these at the top of a function.
 * When the function returns, it will go out of scope, and release the
 * mutex.
 */
typedef Mutex::AutoLock AutoMutex;

#endif

