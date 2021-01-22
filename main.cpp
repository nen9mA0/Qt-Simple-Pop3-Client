#include "pop3client.h"
#include "pop3.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Pop3Client w;
    w.show();
    return a.exec();
}
