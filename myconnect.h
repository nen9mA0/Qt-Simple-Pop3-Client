#ifndef CONNECT_H
#define CONNECT_H

#include <memory>

#include "general.h"

#define RETRY 3

class WSAInit
{
public:
    explicit WSAInit();
    ~WSAInit();
private:
    WSADATA wsadata;
};

class Connect
{
public:
    Connect();
//    Connect(const Connect& rhs);
//    Connect& operator=(const Connect& rhs);
    virtual ~Connect();

    virtual int Send(char* buf, int len) = 0;
    virtual int Recv(char* buf, int len) = 0;

    int last_error;
protected:
    SOCKET connect_socket = INVALID_SOCKET;
    sockaddr_in target;
};

class TCPConnect:public Connect
{
public:
    TCPConnect(char *addr, unsigned int port);
    ~TCPConnect();

    int Init(char *addr, unsigned int port);
    int Connect();
    int Disconnect();
    int Send(char* buf, int len);
    int Recv(char* buf, int buf_len);
    int RecvAll(char* buf, int len);
    int RecvLine(char *buf, int buf_len);
    int SetSendTimeout(int timeout);
    int ResetSendTimeout();
    int SetRecvTimeout(int timeout);
    int ResetRecvTimeout();
    int SetBlock();
    int ResetBlock();
    inline void SetMutex() { use_mutex = true; }
    inline void ResetMutex() { use_mutex = false; }
    inline bool GetStatus() const { return connected; }

    int PrintWSAError(bool print);

private:
    int AcquireTCP();
    int ReleaseTCP();

    bool connected = false;
    bool block = true;
    bool use_mutex = false;
    HANDLE tcp_mutex;
};


extern "C"
{
    DWORD WINAPI ConnectThread(__in  LPVOID lpParameter);
}

#endif // CONNECT_H
