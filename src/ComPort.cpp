

#include <ComPort.h>

ComPort::ComPort(const char *portNode):
        mPortSpeed(ComPort::DEFAULT_PORT_SPEED),
        mPortDataBits(COM_DATA_8_BITS),
        mPortDataParity(COM_PARITY_NONE),
        mPortStopBits(COM_STOP_1_BITS)
{
}

ComPort::ComPort(const char * portNode, int portSpeed, int portDataBits,
        int portDataParity, int portStopBits):
        mPortSpeed(portSpeed),
        mPortDataBits(portDataBits),
        mPortDataParity(portDataParity),
        mPortStopBits(portStopBits)
        
{
}

int ComPort::openPort()
{
    return -1;
}

int ComPort::closePort()
{
    return -1;
}

int ComPort::readPort(const u8 *buf, int len)
{
    return -1;
}

int ComPort::writePort(const u8 *buf, int len)
{
    return -1;
}



