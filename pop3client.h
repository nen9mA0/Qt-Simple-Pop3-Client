#ifndef POP3CLIENT_H
#define POP3CLIENT_H

#include <QMainWindow>

#include "general.h"
#include "myconnect.h"
#include "pop3.h"
#include "mail.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Pop3Client; }
QT_END_NAMESPACE


class Pop3Client : public QMainWindow
{
    Q_OBJECT

public:
    Pop3Client(QWidget *parent = nullptr);
    ~Pop3Client();

    int GetMailLst();       // use LIST command to get mail list
    int GetMail();          // use RETR command to get mails
    int GetOneMail(int index);
    int Interactive();
    int CloseInteractive();

    bool interactive = false;
    bool login_usrname = false;
    bool login_passwd = false;
    TCPConnect *tcp_connect = NULL;
    Ui::Pop3Client *ui;
    Pop *pop_proxy = NULL;
    HANDLE hthread;

signals:
    void Sig_MailGet();
    void Sig_ShowMail(int);

private slots:
    void on_pushButton_clicked();
    void on_Pop3Client_destroyed();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

    void RefreshMailLst();
    void ShowMail(int index);

    void on_listView_clicked(const QModelIndex &index);

private:
    void SetUserBar(bool flag);
    void Reset();

    std::vector<Mail> mail_lst;
    QStandardItemModel item_model;
};


extern "C"
{
    DWORD WINAPI InteractiveThread(__in  LPVOID lpParameter);
}

#endif // POP3CLIENT_H
