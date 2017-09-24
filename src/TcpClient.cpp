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
    mRecvBuf(NULL), mSendBuf(NULL)
    
{
    LOG("TcpClient create default");
}

TcpClient::TcpClient(NetType type) :
    mStop(false),
    mType(type),
    mSocket(-1),
    mStatus(NET_STATUS_DISCONNECT),
    mRecvBuf(NULL), mSendBuf(NULL)
{
    LOG("TcpClient create");
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

bool TcpClient::threadLoop()
{
    int ret = -1;
    mStatus = NET_STATUS_DISCONNECT;

    while (!mStop) {
        logd("tick: %d", get_tick());
        if (NET_STATUS_DISCONNECT == mStatus || mSocket == -1) {
            if (!reConnect()) {
                sleep_ms(500);
                logd("reConnect");
                continue;
            }
        }

        struct timeval tv;
        fd_set fd_read;
        FD_ZERO(&fd_read);
        FD_SET(mSocket, &fd_read);
        tv.tv_sec = 0;
        tv.tv_usec = 500*1000;
        
        ret = select(mSocket + 1, &fd_read, NULL, NULL, &tv);
        if (ret > 0) {
            if (FD_ISSET(mSocket, &fd_read)) {
                if (recvData(mRecvBuf, NET_CAPACITY_RECVBUF) == -1) {
                    loge("ERROR: recvData");
                } else {
                    logd("recvData: %s", mRecvBuf);
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
				loge("Unknown interrupt signal caught\n");
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
    const char *srvIp = "192.200.5.130";
    const unsigned short svrPort = 8234;
    const unsigned int svrNetIp = inet_addr(srvIp);

    do {
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
