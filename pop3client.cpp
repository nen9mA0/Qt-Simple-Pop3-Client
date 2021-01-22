#include "pop3client.h"
#include "ui_pop3client.h"


Pop3Client::Pop3Client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Pop3Client)
    , item_model(this)
{
    ui->setupUi(this);
    SetUserBar(false);
    ui->listView->setModel(&item_model);
    connect(this, SIGNAL(Sig_MailGet()), this, SLOT(RefreshMailLst()));
    connect(this, SIGNAL(Sig_ShowMail(int)), this, SLOT(ShowMail(int)));
}

Pop3Client::~Pop3Client()
{
    CloseInteractive();
    delete ui;
    delete tcp_connect;
    delete pop_proxy;
}

void Pop3Client::SetUserBar(bool flag)
{
    ui->lineEdit->setEnabled(flag);
    ui->lineEdit_2->setEnabled(flag);
    ui->label->setEnabled(flag);
    ui->label_2->setEnabled(flag);
    ui->pushButton_2->setEnabled(flag);
}

int Pop3Client::GetMailLst()
{
    int count;
    char *recv_buf, *p;
    int mail_num;
    int total;
    int mail_index, mail_len;

    pop_proxy->SendList();
    recv_buf = pop_proxy->ServerRecv(&count);
    if(!memcmp(recv_buf, "+OK", 3))
    {
        mail_lst.clear();
        p = recv_buf + 3;
        sscanf(p, "%d %d", &mail_num, &total);
        for(int i=0; i<mail_num; i++)
        {
            while(*p != '\n' && p++);
            p++;
            sscanf(p, "%d %d", &mail_index, &mail_len);
            mail_lst.push_back(Mail(mail_len, mail_index));
        }

        return 0;
    }
    return -1;
}

int Pop3Client::GetMail()
{
    int error;
    int count;
    int i = 0;

    for(std::vector<Mail>::iterator it=mail_lst.begin(); it!=mail_lst.end(); it++)
    {
        int buf_len = it->length+50;
        char *buf = new char[buf_len];
        memset(buf, 0, buf_len);
        error = pop_proxy->SendRetr(it->index);
        tcp_connect->RecvLine(buf, buf_len);

        if(!memcmp(buf, "+OK", 3))
        {
            it->content = buf;
            count = pop_proxy->MailRecv(buf, buf_len);
            it->ProcessMail();
            i++;
        }
        else
        {
            delete [] buf;
        }
    }
    emit Sig_MailGet();
    return i;
}

int Pop3Client::GetOneMail(int index)
{
//    int error;
//    int count;
//    Mail* p = nullptr;

//    if(index < mail_lst.size())
//    {
//        p = &mail_lst[index];
//        if(p->content == nullptr)
//        {
//            char *buffer = new char[p->length+10];
//            memset(buffer, 0, p->length+10);
//            error = pop_proxy->SendRetr(p->index);
//            count = pop_proxy->MailRecv(buffer, p->length);
//            emit Sig_MailGet();
//            return 1;
//        }
//    }
    return 0;
}

void Pop3Client::RefreshMailLst()
{
    QStandardItem *item;
    for(std::vector<Mail>::iterator it = mail_lst.begin(); it != mail_lst.end(); it++)
    {
        if(!it->subject.isNull())
        {
            item = new QStandardItem(it->subject);
            item_model.appendRow(item);
        }
    }
}

void Pop3Client::ShowMail(int index)
{
    Mail *mail = &mail_lst[index];
    QString content("From: " + mail->from + "\nTo: " + mail->to + "\n\n");
    for(std::vector<MailSection*>::iterator it = mail->sections.begin(); it != mail->sections.end(); it++)
    {
        content += (*it)->text + "\n\n";
    }
    ui->textBrowser->setText(content);
}

int Pop3Client::Interactive()
{
    DWORD tid;
    interactive = true;
    tcp_connect->SetMutex();
    hthread = CreateThread(NULL, 0, InteractiveThread, this, 0, &tid);
    return 0;
}

int Pop3Client::CloseInteractive()
{
    if(interactive)
    {
        interactive = false;
        tcp_connect->ResetMutex();
        WaitForSingleObject(hthread, INFINITE);
    }
    return 0;
}

void Pop3Client::on_pushButton_clicked()
{
    TCPConnect *tcp;
    DWORD tid;
    int count;


    qApp->processEvents();
    this->setEnabled(false);

    if(!tcp_connect || !tcp_connect->GetStatus())
    {
        HANDLE hthread_local = CreateThread(NULL, 0, ConnectThread, &tcp, 0, &tid);

        while(1)
        {
            qApp->processEvents();
            if(WaitForSingleObject(hthread_local, 0) == WAIT_OBJECT_0)
                break;
            else
                Sleep(0);
        }
        if( tcp->GetStatus() )
        {
#ifdef DEBUG
            qDebug("Connected");
#endif
            tcp_connect = tcp;
            SetUserBar(true);
            ui->pushButton->setText("Disconnect");
            pop_proxy = new Pop(tcp);
            pop_proxy->ServerRecv(&count);
        }
        else
        {
            delete tcp;
            QMessageBox::warning(NULL,  "warning",  "Connect Error", QMessageBox::Yes);
#ifdef DEBUG
            qDebug("Error");
#endif
        }
    }
    else                    // connected
    {
        tcp = tcp_connect;
        if(tcp->GetStatus())
        {
            Reset();
            ui->pushButton->setText("Connect");
        }
    }
    this->setEnabled(true);
}

void Pop3Client::on_Pop3Client_destroyed()
{

}

void Pop3Client::on_pushButton_2_clicked()
{
    QString username;
    QString passwd;
    int count;

    username = ui->lineEdit->text();
    passwd = ui->lineEdit_2->text();
//    pop_proxy->SendApop(username, passwd);
    pop_proxy->SendUser(username);
    if(!interactive)
    {
        if(!login_usrname)
        {
            pop_proxy->buffer = pop_proxy->ServerRecv(&count);
            if(memcmp(pop_proxy->buffer, "+OK", 3))
                QMessageBox::warning(NULL, "warning", "UserName Error", QMessageBox::Yes, QMessageBox::Yes);
            else
                login_usrname = true;
        }
        if(login_usrname && !login_passwd)
        {
            pop_proxy->SendPassWord(passwd);
            pop_proxy->buffer = pop_proxy->ServerRecv(&count);
            if(memcmp(pop_proxy->buffer, "+OK", 3))
                QMessageBox::warning(NULL, "warning", "PassWd Error", QMessageBox::Yes, QMessageBox::Yes);
            else
                login_passwd = true;
        }
        if(login_usrname && login_passwd)
        {
            if(!GetMailLst())
                GetMail();
        }
    }
    else
    {
        pop_proxy->ServerRecv(&count);
        Sleep(200);
        pop_proxy->SendPassWord(passwd);
        if(!GetMailLst())
            GetMail();
    }
}

void Pop3Client::on_pushButton_3_clicked()
{
    QString input;
    QByteArray bb;
    char buf[BUF_LEN];

    input = ui->lineEdit_3->text();
    bb = input.toUtf8();

    if(strlen(bb.data()) != 0)
    {
        sprintf(buf, "%s\r\n", bb.data());
        tcp_connect->Send(buf, strlen(buf));
        qDebug("send: %s", buf);
    }
}

void Pop3Client::Reset()
{
    CloseInteractive();
    tcp_connect->Disconnect();
    delete tcp_connect;
    tcp_connect = NULL;
    login_passwd = false;
    login_usrname = false;
    SetUserBar(false);
    item_model.clear();
    mail_lst.clear();
}


DWORD WINAPI InteractiveThread(__in  LPVOID lpParameter)
{
    Pop3Client *p = (Pop3Client *)lpParameter;
    TCPConnect *tcp = p->tcp_connect;
    char buf[BUF_LEN+1];

    if(!tcp->ResetBlock())
    {
        while(p->interactive)
        {
            int ret = tcp->Recv(buf, BUF_LEN);
            if(ret == SOCKET_ERROR)
            {
                int err = tcp->last_error;
                if(err == WSAEWOULDBLOCK)
                {
                    Sleep(200);
                }
                else if(err==WSAETIMEDOUT)//超时
                {
                }
                else if(err==WSAENETDOWN)//连接断开
                {
                }
                else  //其他错误
                {
                    continue;
                }
            }
            else
            {
                buf[ret] = '\0';
                qDebug("recv: %d Bytes", strlen(buf));
                qDebug("%s", buf);
            }
        }
        tcp->SetBlock();
    }
    return 0;
}

void Pop3Client::on_listView_clicked(const QModelIndex &index)
{
    int i = index.row();
    emit ShowMail(i);
}
