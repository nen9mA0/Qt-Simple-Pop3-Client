#include "general.h"
#include "mail.h"

Patterns* Patterns::instance = nullptr;
Patterns *pattern = Patterns::InitPattern();

Mail::Mail(int len, int ind, char* buf): length(len), index(ind)
{
    content = new char[len+5];
    memcpy(content, buf, len);
}

Mail::Mail(int len, int ind): length(len), index(ind) { }

Mail::Mail(const Mail& rhs)
{
    index = rhs.index;
    length = rhs.length;
    if(content != nullptr)
    {
        content = new char[length];
        memcpy(content, rhs.content, length);
    }
}

Mail& Mail::operator=(const Mail &rhs)
{
    if(this != &rhs)
    {
        index = rhs.index;
        length = rhs.length;
        if(content != nullptr)
        {
            content = new char[length];
            memcpy(content, rhs.content, length);
        }
    }
    return *this;
}

Mail::~Mail()
{
    if(content != nullptr)
        delete content;
}

int Mail::ProcessMail()
{
    QString mail(content);
    int index=0, sum=0;
    int len = mail.size();

    ProcessHeader(mail);
    while(sum < len)
    {
        mail = mail.mid(index);
        index = ProcessSection(mail);
        sum += index;
        if(index == -1)
            break;
    }

    return 0;
}

int Mail::ProcessHeader(QString &mail)
{
    int pos = pattern->subject_pattern.indexIn(mail);
    if(pos > -1)
    {
        subject = pattern->subject_pattern.cap(1);
    }
    pos = pattern->from_pattern.indexIn(mail);
    if(pos > -1)
    {
        from = pattern->from_pattern.cap(1);
    }
    pos = pattern->to_pattern.indexIn(mail);
    if(pos > -1)
    {
        to = pattern->to_pattern.cap(1);
    }
    return 0;
}

int Mail::ProcessSection(QString &mail)
{
    int pos_sec = pattern->section_pattern.indexIn(mail);
    if(pos_sec > -1)
    {
        QString section = pattern->section_pattern.cap(1);

        int pos = pattern->type_pattern.indexIn(section);
        if(pos > -1)
        {
            QString type = pattern->type_pattern.cap(1);
            if(type.indexOf("text/plain") != -1)
            {
                pos = pattern->text_pattern.indexIn(section);
                if(pos > -1)
                {
                    MailSection *mail_section = new MailSection();
                    mail_section->type = type;
                    QString text = pattern->text_pattern.cap(1);
                    mail_section->text = text;
                    sections.push_back(mail_section);
                }
            }
        }

        return pos_sec + section.size();
    }
    else
    {
        return -1;
    }
}

Patterns::Patterns():
    subject_pattern("Subject: (.*)\r\n"),
    from_pattern("From: (.*)\r\n"),
    to_pattern("To: (.*)\r\n"),
    text_pattern("\r\n\r\n(.*)\r\n\r\n"),
    section_pattern("--_.*\r\n(.*)--_.*\r\n"),
    type_pattern("Content-Type: (.*)\r\n")
{
    subject_pattern.setMinimal(true);
    from_pattern.setMinimal(true);
    to_pattern.setMinimal(true);
    text_pattern.setMinimal(true);
    type_pattern.setMinimal(true);
    section_pattern.setMinimal(true);
}

Patterns* Patterns::InitPattern()
{
    if(instance == nullptr)
        instance = new Patterns;
    return instance;
}
