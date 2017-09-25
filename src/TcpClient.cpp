/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <log.h>
#include <common.h>
#include <TcpClient.h>


#define SEND_RETRY_MAX 5
#define logd LOG
#define loge LOG

TcpClient::TcpClient() :
    mStop(false),
    mType(NET_TYPE_ETHERNET),
    mSocket(-1),
    mStatus(NET_STATUS_DISCONNECT),
    mRecvBuf(NULL), mSendBuf(NULL),
    mLastTick(0),
    mSrvPort(8000)
    
{
    LOG("TcpClient create default");
    memset(mSrvAddr, '\0', sizeof(mSrvAddr));
}

TcpClient::TcpClient(NetType type) :
    mStop(false),
    mType(type),
    mSocket(-1),
    mStatus(NET_STATUS_DISCONNECT),
    mRecvBuf(NULL), mSendBuf(NULL),
    mLastTick(0),
    mSrvPort(8000)
{
    LOG("TcpClient create");
    memset(mSrvAddr, '\0', sizeof(mSrvAddr));
}

TcpClient::~TcpClient()
{
    LOG("TcpClient destroy");
    mStop = true;
    closeConnection();

    delete[] mRecvBuf;
    mRecvBuf = NULL;
    delete[] mSendBuf;
    mSendBuf = NULL;
    LOG("TcpClient destroyed");
}

void TcpClient::onFirstRef()
{
    LOG("TcpClient onFirstRef");
    do {
        if (!mRecvBuf) {
            mRecvBuf = new u8[NET_CAPACITY_RECVBUF];
            if (mRecvBuf == NULL) {
                break;
            }
        }

        if (!mSendBuf) {
            mSendBuf = new u8[NET_CAPACITY_RECVBUF];
            if (mSendBuf == NULL) {
                break;
            }
        }

        return;
    } while (false);

    delete[] mRecvBuf;
    mRecvBuf = NULL;
    delete[] mSendBuf;
    mSendBuf = NULL;
}

/**
 * Format: [2017-09-25 10:00:53]
 */
void TcpClient::getTimestamp(char *buf, int len)
{
    if (buf == NULL || len < strlen("[2017-09-25 10:00:53]")) {
        loge("Error: parameters");
        return;
    }

    time_t now = time(NULL);
    /**
     * struct tm {
     *     int tm_sec;    // Seconds (0-60)
     *     int tm_min;    // Minutes (0-59)
     *     int tm_hour;   // Hours (0-23)
     *     int tm_mday;   // Day of the month (1-31)
     *     int tm_mon;    // Month (0-11)
     *     int tm_year;   // Year - 1900
     *     int tm_wday;   // Day of the week (0-6, Sunday = 0)
     *     int tm_yday;   // Day in the year (0-365, 1 Jan = 0)
     *     int tm_isdst;  // Daylight saving time
     * };
     */
    struct tm *tm = localtime(&now);
    snprintf(buf, len, "[%d-%02d-%02d %02d:%02d:%02d]",
        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

bool TcpClient::isValidIpAddr(const char *addr)
{
    if (addr == NULL) return false;
    if (strcmp(addr, "255.255.255.255") == 0) {
        return true;
    } else {
        return inet_addr(addr) != INADDR_NONE;
    }

    return false;
}

void TcpClient::setServer(const char *srvAddr, u16 port)
{
    if (srvAddr == NULL || !isValidIpAddr(srvAddr)) {
        loge("Error: parameter");
        return;
    }

    strncpy(mSrvAddr, srvAddr, sizeof(mSrvAddr) - 1);
    mSrvPort = port;
}

bool TcpClient::threadLoop()
{
    int ret = -1;
    mStatus = NET_STATUS_DISCONNECT;

    while (!mStop) {
        if (NET_STATUS_DISCONNECT == mStatus || mSocket == -1) {
            if (!reConnect()) {
                sleep_ms(500);
                logd("reConnect");
                continue;
            }
        }

        struct timeval tv;
        fd_set fd_read;
        fd_set fd_write;

        FD_ZERO(&fd_read);
        FD_ZERO(&fd_write);

        FD_SET(mSocket, &fd_read);
        FD_SET(mSocket, &fd_write);

        tv.tv_sec = 0;
        tv.tv_usec = 50*1000;
        ret = select(mSocket + 1, &fd_read, NULL, NULL, &tv);
        if (ret >= 0) {
            if (FD_ISSET(mSocket, &fd_read)) {
                /**
                 * Handle data recv
                 */
                ret = recvData(mRecvBuf, NET_CAPACITY_RECVBUF);
                if (ret > 0) {
                    logd("recvData: %s", mRecvBuf);
                } else if (ret == -1) { // error
                    loge("ERROR: recvData");
                } else {
                    // Maybe server has closed
                    //logd("recvData return: %d", ret);
                }
            } else if (FD_ISSET(mSocket, &fd_write)) {
                /**
                 * Handle data write
                 */
                if ((get_tick() - mLastTick) > TICK_PERIOD) {
                    mLastTick = get_tick();
                    sendHeartBeatMsg();
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

			//goto terminate;
        }
    }

    return true;
}

/**
 * Socket
 */
bool TcpClient::openConnection(const char *srvIpAddr, u16 srvPort)
{
    bool ret = false;
    int sock = -1;
    do {
        /**
         * On success, a file descriptor for the new socket is returned.  On error, -1 is returned, and errno is set appropriately
         */
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            loge("ERROR: socket create, %s", strerror(errno));
            break;
        }

        struct sockaddr_in sockAddr;
        memset(&sockAddr, 0, sizeof(struct sockaddr_in));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_port = htons(srvPort);
        sockAddr.sin_addr.s_addr = inet_addr(srvIpAddr);

        /**
         * On error, -1 is returned, if the connection or binding succeeds, zero is returned
         */
        if (connect(sock, (struct sockaddr *)&sockAddr, sizeof(struct sockaddr_in)) == -1) {
            loge("ERROR: connect error %s", strerror(errno));
            break;
        }

        ret = true;
        mSocket = sock;
        mStatus = NET_STATUS_CONNECTED;
    } while (0);

    return ret;
}

bool TcpClient::closeConnection()
{
    bool ret = true;
    return ret;
}

bool TcpClient::reConnect()
{
    bool ret = false;
    const char *ifName = "eth0";
    const char *srvIp = mSrvAddr;
    const unsigned short svrPort = mSrvPort;

    do {
        if (!isValidIpAddr(srvIp)) {
            loge("ERROR: invalid ip address");
            break;
        } else {
            const unsigned int svrNetIp = inet_addr(srvIp);
        }

        if (!closeConnection()) {
            loge("ERROR: closeConnection");
            break;
        }

#if 0 //	OS_FAMILY_UNIX
        if (!have_route_to(svrNetIp, ifName)) {
            logd(" tm=%d: **no** have_route_to(%s, %s)\r\n", get_tick(), srvIp, ifName);
            ret= (setIfHostRoute(svrNetIp, ifName) > 0);
            if (ret) {
                logd("tm=%d: **ok** setIfRoute(%s, %s)\r\n", get_tick(), srvIp, ifName);
            }
        } else
#endif
        {
            ret = true;
            logd("tm=%d: **yes** have_route_to(%s, %s)\r\n", get_tick(), srvIp, ifName);
        }

        if (ret) {
            ret = openConnection(srvIp, svrPort);
        }
    } while (0);

    if (ret) {
        mStatus = NET_STATUS_CONNECTED;
        logd("tm=%d: **yes** openConnection(%s, %s)\r\n", get_tick(), srvIp, ifName);
    } else {
        mStatus = NET_STATUS_DISCONNECT;
        loge("tm=%d: **no** openConnection(%s, %s)\r\n", get_tick(), srvIp, ifName);
    }

    return ret;
}

/**
 * Socket data
 */
int TcpClient::sendData(const u8 *buf, int len)
{
    if (buf == NULL || len <= 0) {
        loge("Error: parameters");
    }

    int written = 0, last;
    int retry = 0;
    int ret = 0;
    while (written != len && retry < SEND_RETRY_MAX) {
        /**
         * On success, these calls return the number of bytes sent
         * On error, -1 is returned, and errno is set appropriately
         */
        last = send(mSocket, (const void *)(buf + written), len - written, 0);
        if (last == -1) {
            loge("Error: send %s", strerror(errno));
            return -1;
        }

        written += last;
        retry ++;
        sleep_ms(50);
    }

    if (retry == SEND_RETRY_MAX) {
        ret = -1;
        loge("Error: sendData retry expires");
    }

    return ret;
}

int TcpClient::recvData(const u8 *buf, int len)
{
    int ret = -1;
    if (buf == NULL || len <= 0) {
        loge("Error: parameters");
    }
    memset((void *)buf, 0, len);
    /**
     * ssize_t recv(int sockfd, void *buf, size_t len, int flags);
     * return the number of bytes received, or -1 if an error occurred
     */
    ret = recv(mSocket, (void *) buf, len, 0);
    if (ret == -1) {
        loge("Error: recv %s", strerror(errno));
        mStatus = NET_STATUS_DISCONNECT;
    }

    return ret;
}

bool TcpClient::recvMsgPacket()
{
    bool ret = false;
    return ret;
}

bool TcpClient::sendMsgPacket()
{
    bool ret = false;
    return ret;
}

bool TcpClient::isHeartBeatErr()
{
    bool ret = false;
    return ret;
}

int TcpClient::sendHeartBeatMsg()
{
    char timeStamp[32] = {'\0'};
    char sendBuf[64] = {'\0'};
    getTimestamp(timeStamp, sizeof(timeStamp) - 1);
    snprintf(sendBuf, sizeof(sendBuf) - 1, "%s heartbeat.", timeStamp);
    sendData((const u8 *)sendBuf, strlen(sendBuf));
    return 0;
}

