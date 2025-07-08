#include "clientlogic.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QString host = "127.0.0.1";
    quint16 port_1 = 12345;
    quint16 port_2 = 12346;

    QList<ClientLogic *> clients;
    for (int i = 0; i < 3; ++i) {
        clients.append(new ClientLogic(host, port_1, &a));
        clients.last()->start();
    }
    for (int i = 0; i < 3; ++i) {
        clients.append(new ClientLogic(host, port_2, &a));
        clients.last()->start();
    }

    return a.exec();
}
