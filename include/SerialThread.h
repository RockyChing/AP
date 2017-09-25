#ifndef _LINUX_SERIAL_THREAD_H
#define _LINUX_SERIAL_THREAD_H
#include <Thread.h>
#include <log.h>
#include <common.h>
#include <ComPort.h>


class SerialThread: public Thread {
public:
     SerialThread();
    ~SerialThread();

private:
    bool threadLoop();
    void onFirstRef();

private:
    sp<ComPort> mSerial;
};




#endif

