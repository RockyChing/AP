#ifndef _LINUX_COMPORT_H
#define _LINUX_COMPORT_H

#include <types.h>


class ComPort {
    static const int DEFAULT_PORT_SPEED = 9600;


private:
    enum ComDataBits {
        COM_DATA_4_BITS = 7,
        COM_DATA_5_BITS = 7,
        COM_DATA_6_BITS = 7,
        COM_DATA_7_BITS = 7,
        COM_DATA_8_BITS = 8,
    };

    enum ComParity {
        COM_PARITY_NONE = 0,
        COM_PARITY_ODD  = 1,
        COM_PARITY_EVEN = 2,
    };

    enum ComStopBits {
        COM_STOP_1_BITS   = 1,
        COM_STOP_1_5_BITS = 1,
        COM_STOP_2_BITS   = 2,
    };

public:
    ComPort(const char *portNode);
    ComPort(const char *portNode, int portSpeed,
        int portDataBits, int portDataParity, int portStopBits);

     int openPort();
     int closePort();
     int readPort(const u8 *buf, int len);
     int writePort(const u8 *buf, int len);

// disable default constructor & copy construct & operator =
private:
    ComPort();
    ComPort(const ComPort &);
    ComPort& operator=(const ComPort &);

private:
    int mFd;
    int mPortSpeed;
    int mPortDataBits;
    int mPortDataParity;
    int mPortStopBits;
};















#endif

