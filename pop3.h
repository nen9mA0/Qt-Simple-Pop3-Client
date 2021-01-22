#pragma once

#include "general.h"
#include "myconnect.h"

class Pop
{
public:
    explicit Pop(TCPConnect *socket);
    ~Pop();

    inline char* GetBuf() { return buffer; }
    char* ServerRecv(int *count);
    int BufRecv(char *buf, int len);
    int MailRecv(char *buf, int len);
    int SendUser(QString &username);
    int SendPassWord(QString &passwd);
    int SendApop(QString &username, QString &passwd);
    int SendList();
    int SendRetr(int index);

    inline int TCPSetBlock();
    inline int TCPResetBlock();

    char* buffer;
private:
    TCPConnect *tcp_socket;
    std::string document;
};
