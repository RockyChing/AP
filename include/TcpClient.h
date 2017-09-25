#ifndef _LINUX_TCP_CLIENT_H
#define _LINUX_TCP_CLIENT_H

#include <Thread.h>
#include <log.h>
#include <common.h>

class TcpClient : public Thread {
    static const u32 TICK_PERIOD = 10 * 1000; // ms
    static void getTimestamp(char *buf, int len);
    static bool isValidIpAddr(const char *addr);

public:
    enum NetType {
        NET_TYPE_MOBILE,
        NET_TYPE_WIFI,
        NET_TYPE_ETHERNET,
        NET_TYPE_INVALID
    };

     TcpClient();
     TcpClient(NetType type);
    ~TcpClient();

    void setServer(const char *srvAddr, u16 port);

// private methods
private:
    bool threadLoop();
    void onFirstRef();

    /**
     * Socket
     */
    bool openConnection(const char *srvIpAddr, u16 srvPort);
    bool closeConnection();
    bool reConnect();

    /**
     * Socket data
     */
    int sendData(const u8 *buf, int len);
    int recvData(const u8 *buf, int len);
    bool recvMsgPacket();
    bool sendMsgPacket();
    bool isHeartBeatErr();

    int sendHeartBeatMsg();

// disable cope construct & operator =
private:
    TcpClient(const TcpClient&);
    TcpClient& operator=(const TcpClient&);

// private members
private:
    enum NetStatus {
        NET_STATUS_DISCONNECT,
        NET_STATUS_CONNECTED,
        NET_STATUS_HEART_BEAT,
        NET_STATUS_RUNNING
    };

    enum {
        NET_CAPACITY_SENDBUF = 4 << 10,
        NET_CAPACITY_RECVBUF = 4 << 10,
    };

    volatile bool mStop;
    NetType mType;
    int mSocket;
	NetStatus  mStatus;

	u8 *mRecvBuf;
    u8 *mSendBuf;

    u32 mLastTick;

    char mSrvAddr[32];
    u16  mSrvPort;
};


#endif

