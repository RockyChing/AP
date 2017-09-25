#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <log.h>
#include <common.h>
#include <ComPort.h>

#define logd LOG
#define loge LOG

struct BaudAlias {
    int baud;
    int baudalias;
};

struct BaudAlias BaudArray[] {
    { 0,      B0 },
    { 50,     B50 },
    { 75,     B75 },
    { 110,    B110 },
    { 134,    B134 },
    { 150,    B150 },
    { 200,    B200 },
    { 300,    B300 },
    { 600,    B600 },
    { 1200,   B1200 },
    { 1800,   B1800 },
    { 2400,   B2400 },
    { 4800,   B4800 },
    { 9600,   B9600 },
    { 19200,  B19200 },
    { 38400,  B38400 },
    { 57600,  B57600 },
    { 115200, B115200 },
    { 230400, B230400 },
};

ComPort::ComPort(const char *portName):
        mPortName(portName),
        mFd(-1),
        mPortSpeed(ComPort::DEFAULT_PORT_SPEED),
        mPortDataBits(COM_DATA_8_BITS),
        mPortDataParity(COM_PARITY_NONE),
        mPortStopBits(COM_STOP_1_BITS)
{
}

ComPort::ComPort(const char *portName, int portSpeed, int portDataBits,
        int portDataParity, int portStopBits):
        mPortName(portName),
        mFd(-1),
        mPortSpeed(portSpeed),
        mPortDataBits(portDataBits),
        mPortDataParity(portDataParity),
        mPortStopBits(portStopBits)
        
{
}

ComPort::~ComPort()
{
    closePort();
}

int ComPort::init(int fd)
{
    int fdflags;
	ASSERT(fdflags = fcntl(fd, F_GETFL));
	ASSERT(fcntl(fd, F_SETFL, fdflags & ~O_NONBLOCK));
    struct termios options;

    /**
     * Return 0 on success, -1 on failure and set errno to indicate the error
     */
    if (tcgetattr(fd, &options) != 0) {
        loge("Error: terminal tcgetattr");
        return -1;
    }

    int baudAlias = getBaudAlias(mPortSpeed);
    cfsetispeed(&options,  baudAlias);
    cfsetospeed(&options,  baudAlias);
    switch (mPortDataBits) {
        case COM_DATA_8_BITS:
            options.c_cflag |= CS8;
            break;
        case COM_DATA_7_BITS:
            options.c_cflag |= CS7;
            break;
        case COM_DATA_6_BITS:
            options.c_cflag |= CS6;
            break;
        case COM_DATA_5_BITS:
            options.c_cflag |= CS5;
            break;
        default:
            loge("Error: Unsupported terminal data bits!\n");
            return -1;
    }

    switch (mPortDataParity) {
        case COM_PARITY_NONE:
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case COM_PARITY_ODD:
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case COM_PARITY_EVEN:
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        default:
            loge("Error: Unsupported terminal parity!\n");
            return -1;
    }

    switch (mPortStopBits) {
        case COM_STOP_1_BITS:
        case COM_STOP_1_5_BITS:
            options.c_cflag &= ~CSTOPB;
            break;
        case COM_STOP_2_BITS:
            options.c_cflag |= CSTOPB;
            break;
        default:
            loge("Error: Unsupported terminal parity!\n");
            return -1;
    }

    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
    options.c_oflag  &= ~(OPOST | OCRNL);
    options.c_iflag &= ~(IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL);
    options.c_iflag &= ~(IXON | IXOFF | BRKINT);
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        loge("Error: terminal tcsetattr");
        return -1;
    }

    return 0;
}

int ComPort::getBaudAlias(int speed)
{
    s16 i;
    for (i = NUM_ELEMENTS(BaudArray) - 1; i >= 0; i --) {
        if (speed == BaudArray[i].baud)
            return BaudArray[i].baudalias;
    }

    loge("Unsupport baud for %s, use %d as default", mPortName ? mPortName : "NULL",
            DEFAULT_PORT_SPEED);
    return DEFAULT_PORT_SPEED;
}

int ComPort::openPort()
{
    if (mPortName == NULL) {
        loge("Error: open null port");
        return -1;
    }

    int ret = open(mPortName, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (ret <= 0) {
        loge("Error: open port");
        return -1;
    }

    /**
     * isatty() returns 1 if fd is an open file descriptor referring to a terminal;
     * otherwise 0 is returned, and errno is set to indicate the error
     */
    if (!isatty(ret)) {
        loge("Error: %s is not a terminal", mPortName);
    }

    mFd = ret;
    if (init(mFd) < 0) {
        loge("Error: init port");
        return -1;
    }

    logd("%s open done, fd = %d", mPortName, mFd);
    return -1;
}

void ComPort::closePort()
{
    mPortName = NULL;
    if (mFd > 0) {
        close(mFd);
        mFd = -1;
    }
}

int ComPort::readPort(u8 *buf, int len)
{
    if (mFd <= 0) {
        loge("Error: invalid fd");
        return -1;
    }

    int readBytes = read(mFd, buf, len -1);
    if (readBytes > 0) {
        buf[readBytes] = '\0';
        dump("<s ", buf, readBytes);
    } else if (readBytes < 0) {
        loge("Error: %s", strerror(errno));
    } else {
    }

    return readBytes;
}

int ComPort::writePort(const u8 *buf, int len)
{
    if (mFd <= 0) {
        loge("Error: invalid fd");
        return -1;
    }

    int writeBytes = write(mFd, buf, len);
    if (writeBytes < 0) {
        loge("Error: %s", strerror(errno));
    }
    return writeBytes;
}

int ComPort::getFd() const
{
    return mFd;
}

int ComPort::dump(const char * prefix, const u8 * ptr, u32 length)
{
    char buffer[100] = {'\0'};
    u32  offset = 01;
    int  i;
    AutoMutex _l(mLock);

    while (offset < length) {
        int off;
        strcpy(buffer, prefix);
        off = strlen(buffer);
        ASSERT(snprintf(buffer + off, sizeof(buffer) - off, "%08x: ", offset));
        off = strlen(buffer);

        for (i = 0; i < 16; i ++) {
            if (offset + i < length) {
                ASSERT(snprintf(buffer + off, sizeof(buffer) - off, "%02x%c", ptr[offset + i], i == 7 ? '-' : ' '));
            } else {
                ASSERT(snprintf(buffer + off, sizeof(buffer) - off, " .%c", i == 7 ? '-' : ' '));
            }
            off = strlen(buffer);
        }

        ASSERT(snprintf(buffer + off, sizeof(buffer) - off, " "));
		off = strlen(buffer);
		for (i = 0; i < 16; i++)
			if (offset + i < length) {
				ASSERT(snprintf(buffer + off, sizeof(buffer) - off, "%c", (ptr[offset + i] < ' ') ? '.' : ptr[offset + i]));
				off = strlen(buffer);
			}

        offset += 16;
		LOG("== %s", buffer);
    }

    return 0;
}

