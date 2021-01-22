#ifndef MAIL_H
#define MAIL_H

#include "general.h"

typedef struct _MailSection
{
    QString text;
    QString type;
} MailSection;

class Mail
{
public:
    Mail(int len, int ind, char* buf);
    Mail(int len, int ind);

    Mail(const Mail& rhs);
    Mail& operator=(const Mail& rhs);

    ~Mail();

    int ProcessMail();
    int ProcessHeader(QString &mail);
    int ProcessSection(QString &mail);

    int index;
    int length;
    char *content = nullptr;

    QString subject;
    QString from;
    QString to;
    std::vector<MailSection*> sections;
};


class Patterns
{
public:
    static Patterns* InitPattern();

    QRegExp subject_pattern;
    QRegExp from_pattern;
    QRegExp to_pattern;
    QRegExp text_pattern;
    QRegExp section_pattern;
    QRegExp type_pattern;
private:
    Patterns();
    Patterns(const Patterns&);
    Patterns& operator=(const Patterns&);


    static Patterns* instance;
};

#endif // MAIL_H
