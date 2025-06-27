#include <QCoreApplication>
#include "clientlogic.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString host = "127.0.0.1";
    quint16 port = 12345;

    ClientLogic client(host, port, &a);
    client.start();

    return a.exec();
}
