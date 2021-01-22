#include "pop3.h"

Pop::Pop(TCPConnect *socket): tcp_socket(socket)
{
    buffer = new char[BUF_LEN];
}

Pop::~Pop()
{
    delete [] buffer;
}

char* Pop::ServerRecv(int *count)
{
    *count = tcp_socket->Recv(buffer, BUF_LEN);
#ifdef DEBUG
    qDebug("Server: %s", buf);
#endif
    return buffer;
}

int Pop::BufRecv(char *buf, int len)
{
    int count = tcp_socket->Recv(buf, len);
#ifdef DEBUG
    qDebug("Server: %s", buf);
#endif
    return count;
}

int Pop::MailRecv(char *buf, int len)
{
    int count;

    tcp_socket->SetRecvTimeout(200);
    count = tcp_socket->RecvAll(buf, len);
    tcp_socket->ResetRecvTimeout();
    if(count != len)
        return -1;
#ifdef DEBUG
    qDebug("Server: %s", buf);
#endif
    return count;
}

int Pop::SendUser(QString &username)
{
    QByteArray tmp = username.toUtf8();
    sprintf(buffer, "user %s\r\n", tmp.data());
    int count = tcp_socket->Send(buffer, strlen(buffer));
    return count;
}

int Pop::SendPassWord(QString &passwd)
{
    QByteArray tmp = passwd.toUtf8();
    sprintf(buffer, "pass %s\r\n", tmp.data());
    int count = tcp_socket->Send(buffer, strlen(buffer));
    return count;
}

int Pop::SendApop(QString &username, QString &passwd)
{
    QByteArray bb;
    QByteArray qpass = passwd.toUtf8();
    QByteArray quser = username.toUtf8();
    bb = QCryptographicHash::hash ( qpass, QCryptographicHash::Md5 );
    sprintf(buffer, "apop %s,%s\r\n", quser.data(), bb.toHex().data());
    int count = tcp_socket->Send(buffer, strlen(buffer));
    return count;
}

int Pop::SendList()
{
    sprintf(buffer, "list\r\n");
    int count = tcp_socket->Send(buffer, strlen(buffer));
    return count;
}

int Pop::SendRetr(int index)
{
    sprintf(buffer, "retr %d\r\n", index);
    int count = tcp_socket->Send(buffer, strlen(buffer));
    return count;
}

inline int Pop::TCPSetBlock()
{
    return tcp_socket->SetBlock();
}

inline int Pop::TCPResetBlock()
{
    return tcp_socket->ResetBlock();
}
