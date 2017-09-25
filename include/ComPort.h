#ifndef _LINUX_COMPORT_H
#define _LINUX_COMPORT_H

#include <types.h>
#include <Mutex.h>
#include <RefBase.h>

class ComPort : public RefBase {
    static const int DEFAULT_PORT_SPEED = 115200;


private:
    enum ComDataBits {
        COM_DATA_5_BITS = 5,
        COM_DATA_6_BITS = 6,
        COM_DATA_7_BITS = 7,
        COM_DATA_8_BITS = 8,
    };

    enum ComParity {
        COM_PARITY_NONE = 0,
        COM_PARITY_ODD  = 1,
        COM_PARITY_EVEN = 2,
    };

    enum ComStopBits {
        COM_STOP_1_BITS   = 0,
        COM_STOP_1_5_BITS = 1,
        COM_STOP_2_BITS   = 2,
    };

public:
    ComPort(const char *portName);
    ComPort(const char *portName, int portSpeed,
        int portDataBits, int portDataParity, int portStopBits);
    ~ComPort();

     int openPort();
     int readPort(u8 *buf, int len);
     int writePort(const u8 *buf, int len);
     int getFd() const;

// disable default constructor & copy construct & operator =
private:
    ComPort();
    ComPort(const ComPort &);
    ComPort& operator=(const ComPort &);
    void closePort();

    int init(int fd);
    int getBaudAlias(int speed);
    int dump(const char *prefix, const u8 *ptr, u32 length);

private:
    const char *mPortName;

    int mFd;
    int mPortSpeed;
    int mPortDataBits;
    int mPortDataParity;
    int mPortStopBits;

    Mutex mLock;
};















#endif

