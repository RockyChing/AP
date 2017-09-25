#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <SerialThread.h>

#define logd LOG
#define loge LOG

SerialThread::SerialThread()
{
    logd("SerialThread create");
}

SerialThread::~SerialThread()
{
    logd("SerialThread destroy");
}

void SerialThread::onFirstRef()
{
    mSerial = new ComPort("/dev/mux2");
    mSerial->openPort();
}

bool SerialThread::threadLoop()
{
    int ret = -1;
    u8 buf[512];
    const int fd = mSerial->getFd();

    while (1) {
        struct timeval tv;
        fd_set fd_read;
        FD_ZERO(&fd_read);
        FD_SET(fd, &fd_read);

        tv.tv_sec = 0;
        tv.tv_usec = 50*1000;
        ret = select(fd + 1, &fd_read, NULL, NULL, NULL);
        if (ret >= 0) {
            if (FD_ISSET(fd, &fd_read)) {
                ret = mSerial->readPort(buf, sizeof(buf));
                if (ret > 0) {
                    loge("%d bytes read", ret);
                } else {
                    loge("ret = %d", ret);
                }
            }
        } else {
            switch (errno){
			case EINTR:
				loge("Interrupt signal EINTR caught");
				break;
			case EAGAIN:
				loge("Interrupt signal EAGAIN caught");
				break;
			case EBADF:
				loge("Interrupt signal EBADF caught");
				break;
			case EINVAL:
				loge("Interrupt signal EINVAL caught");
				break;
			default:
				loge("Interrupt signal %d caught, it says \"%s\"", errno, strerror(errno));
			}
        }
    }
    return true;
}
