#include "myconnect.h"

char server_addr[] = "123.126.97.79";     // pop3.163.com
unsigned int port = 110;

//char server_addr[] = "127.0.0.1";     // pop3.163.com
//unsigned int port = 10001;
static WSAInit wsa_init;


WSAInit::WSAInit()
{
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (iResult != NO_ERROR)
    {
#ifdef DEBUG
        qDebug("WSAStartup failed with error: %ld\n", iResult);
#endif //
    }

#ifdef INIT_DEBUG
    qDebug("WSAInit init\n");
#endif
}

WSAInit::~WSAInit()
{
    WSACleanup();
#ifdef INIT_DEBUG
    qDebug("WSAInit deinit\n");
#endif
}

Connect::Connect()
{
#ifdef INIT_DEBUG
    qDebug("Connect init\n");
#endif
}

//Connect::Connect(const Connect& rhs)
//{
//#ifdef INIT_DEBUG
//    qDebug("Connect copy\n");
//#endif
//    pingpong_buf[0] = new char[BUF_LEN * 2];
//    pingpong_buf[1] = pingpong_buf[0] + BUF_LEN;

//    memcpy(pingpong_buf, rhs.pingpong_buf, BUF_LEN * 2);

//}

//Connect& Connect::operator=(const Connect &rhs)
//{
//#ifdef INIT_DEBUG
//    qDebug("Connect operator=\n");
//#endif
//}

Connect::~Connect()
{
#ifdef INIT_DEBUG
    qDebug("Connect deinit\n");
#endif
}

TCPConnect::~TCPConnect()
{
    if (GetStatus())
        Disconnect();
    if (connect_socket != INVALID_SOCKET)
        closesocket(connect_socket);
#ifdef INIT_DEBUG
    qDebug("TCPConnect deinit\n");
#endif
}

TCPConnect::TCPConnect(char *addr, unsigned int port)
{
    int iResult = Init(addr, port);
    if (iResult != 0)
    {
#ifdef DEBUG
        qDebug("TCPConnect Init failed: %d\n", last_error);
#endif
    }
#ifdef INIT_DEBUG
    qDebug("TCPConnect init\n");
#endif
}

int TCPConnect::Init(char *addr, unsigned int port)
{
    connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect_socket == INVALID_SOCKET)
    {
        PrintWSAError(PRINTWSAERR);
#ifdef DEBUG
        qDebug("Error at socket(): %ld\n", last_error);
#endif
    }
    else
    {
        target.sin_family = AF_INET;
        target.sin_addr.s_addr = inet_addr(addr);
        target.sin_port = htons(port);
        last_error = 0;
    }
    return last_error;
}

int TCPConnect::Connect()
{
    int iResult = connect(connect_socket, (SOCKADDR*)&target, sizeof(target));
    if (iResult == SOCKET_ERROR)
    {
        PrintWSAError(PRINTWSAERR);
        connected = false;
#ifdef DEBUG
        qDebug("Unable to connect to server: %ld\n", last_error);
#endif
    }
    else
    {
        last_error = 0;
        connected = true;
    }
    return last_error;
}

int TCPConnect::Disconnect()
{
    if (connected)
    {
        closesocket(connect_socket);
        connected = false;
    }
#ifdef INIT_DEBUG
        qDebug("TCPConnect Disconnect");
#endif
    return 0;
}

int TCPConnect::Send(char* buf, int len)
{
    int count = 0;
    char *p = buf;
#ifdef DEBUG
    char tmp[BUF_LEN*2+10];
    sprintf(tmp, "Send: ");
    for(int i=0; i<len; i++)
        sprintf(tmp+6+2*i, "%02X", p[i]);
    tmp[len*2+6] = '\0';
    qDebug("%s", tmp);
    qDebug("%s", p);
#endif

    AcquireTCP();
    while (count < len)
    {
        int ret = send(connect_socket, p, len - count, 0);
        if (ret == SOCKET_ERROR)
        {
            PrintWSAError(PRINTWSAERR);
            break;
#ifdef DEBUG
            qDebug("Error when sending message at position %d, Message: %10s ...\n", count, p);
#endif
        }
        else
        {
            last_error = 0;
            count += ret;
            p = &buf[count];
        }

    }
    ReleaseTCP();
    return count;
}

int TCPConnect::Recv(char *buf, int buf_len)
{
    int ret = 0;
    char *p = buf;

    AcquireTCP();
    ret = recv(connect_socket, p, buf_len, 0);
    if(ret == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        if(err == WSAEWOULDBLOCK)
        {
            last_error = err;
        }
        else if(err == WSAETIMEDOUT)
        {
            last_error = err;
        }
        else
        {
            PrintWSAError(PRINTWSAERR);
        }
    }
#ifdef DEBUG
    char tmp[BUF_LEN*2+10];
    if(count != SOCKET_ERROR)
    {
        sprintf(tmp, "Recv: ");
        for(int i=0; i<count; i++)
            sprintf(tmp+2*i+6, "%02X", p[i]);
        tmp[len*2+6] = '\0';
        p[count] = '\0';
        qDebug("%s", tmp);
        qDebug("%s", p);
    }
#endif
    ReleaseTCP();
    return ret;
}

int TCPConnect::RecvAll(char *buf, int len)
{
    int count = 0;
    int ret = 0;
    char *p = buf;
    AcquireTCP();
    while(ret < len)
    {
        ret = recv(connect_socket, p, len-count, 0);
        if(ret == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            if(err == WSAEWOULDBLOCK)
            {
                last_error = err;
            }
            else if(err == WSAETIMEDOUT)
            {
                last_error = err;
                break;
            }
            else
            {
                PrintWSAError(PRINTWSAERR);
                break;
            }
        }
        else
        {
            count += ret;
            p += ret;
        }
#ifdef DEBUG
        char tmp[BUF_LEN*2+10];
        if(count != SOCKET_ERROR)
        {
            sprintf(tmp, "Recv: ");
            for(int i=0; i<count; i++)
                sprintf(tmp+2*i+6, "%02X", p[i]);
            tmp[len*2+6] = '\0';
            p[count] = '\0';
            qDebug("%s", tmp);
            qDebug("%s", p);
        }
#endif
    }
    ReleaseTCP();
    return count;
}

int TCPConnect::RecvLine(char *buf, int buf_len)
{
    int count = 0;
    char *p = buf;
    AcquireTCP();
    while(count < buf_len)
    {
        int ret = recv(connect_socket, p, 1, 0);
        if(ret == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            if(err == WSAEWOULDBLOCK)
            {
                last_error = err;
            }
            else if(err == WSAETIMEDOUT)
            {
                last_error = err;
                break;
            }
            else
            {
                PrintWSAError(PRINTWSAERR);
                break;
            }
        }
        count++;
        if(*p == '\n')
            break;
        p++;
    }
    *(p+1) = '\0';
    ReleaseTCP();
    return count;
}

int TCPConnect::SetSendTimeout(const int timeout)
{
    if(setsockopt(connect_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        return PrintWSAError(PRINTWSAERR);
    return 0;
}

int TCPConnect::ResetSendTimeout()
{
    const int timeout = 0;
    if(setsockopt(connect_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        return PrintWSAError(PRINTWSAERR);
    return 0;
}

int TCPConnect::SetRecvTimeout(const int timeout)
{
    if(setsockopt(connect_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        return PrintWSAError(PRINTWSAERR);
    return 0;
}

int TCPConnect::ResetRecvTimeout()
{
    const int timeout = 0;
    if(setsockopt(connect_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        return PrintWSAError(PRINTWSAERR);
    return 0;
}

DWORD WINAPI ConnectThread(__in  LPVOID lpParameter)
{
    TCPConnect *tcp = new TCPConnect(server_addr, port);
    int i = 0;
    while (tcp->last_error != 0 && i<RETRY)
    {
#ifdef DEBUG
        qDebug("Retry...\n");
#endif // DEBUG
        i++;
        tcp->Init(server_addr, port);
        Sleep(1000);
    }

    if(tcp->last_error == 0)
    {
        i = 0;
        tcp->Connect();
        while (!tcp->GetStatus() && i<RETRY)
        {
#ifdef DEBUG
            qDebug("Connect Error, error code: %d", tcp->last_error);
#endif
            i++;
            Sleep(1000);
            tcp->Connect();
#ifdef DEBUG
            qDebug("Retry...\n");
#endif // DEBUG
        }
    }

    *reinterpret_cast<TCPConnect **>(lpParameter) = tcp;    // Attension: pointer doesn't be deleted here
    return 0;
}

int TCPConnect::ResetBlock()
{
    unsigned long mode = 1;

    if(block)
    {
        int ret = ioctlsocket(connect_socket, FIONBIO, &mode);
        if(ret == SOCKET_ERROR)
        {
            PrintWSAError(PRINTWSAERR);
            return -1;
        }
        block = false;
    }
    return 0;
}

int TCPConnect::SetBlock()
{
    unsigned long mode = 0;

    if(!block)
    {
        int ret = ioctlsocket(connect_socket, FIONBIO, &mode);
        if(ret == SOCKET_ERROR)
        {
            PrintWSAError(PRINTWSAERR);
            return -1;
        }
        block = true;
    }
    return 0;
}

int TCPConnect::AcquireTCP()
{
    if(use_mutex)
        WaitForSingleObject(tcp_mutex, INFINITE);
    return 0;
}

int TCPConnect::ReleaseTCP()
{
    if(use_mutex)
        ReleaseMutex(tcp_mutex);
    return 0;
}

int TCPConnect::PrintWSAError(bool print)
{
    int err = WSAGetLastError();
    wchar_t s[256];

    last_error = err;
    if(print)
    {
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, err,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&s, sizeof(s), NULL);
        qDebug("%s", s);
    }
    return SOCKET_ERROR;
}
