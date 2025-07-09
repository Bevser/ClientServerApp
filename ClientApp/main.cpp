#include <QCoreApplication>
#include <QDebug>
#include <QStringConverter>
#include <QLoggingCategory>
#include <iostream>

#include "clientlogic.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // Значения по умолчанию
    QString host = "127.0.0.1";
    quint16 port = 12345;
    int clientCount = 3;

    // Обработка аргументов командной строки
    QStringList args = a.arguments();
    for (const QString &arg : args) {
        if (arg.startsWith("--port=")) {
            bool ok;
            quint16 p = arg.mid(QString("--port=").length()).toUShort(&ok);
            if (ok) {
                port = p;
            }
        } else if (arg.startsWith("--clients=")) {
            bool ok;
            int count = arg.mid(QString("--clients=").length()).toInt(&ok);
            if (ok) {
                clientCount = count;
            }
        }
    }

    // Вывод информации о запуске
    qDebug() << QString("Start %1 clients to (%2:%3).")
                    .arg(clientCount)
                    .arg(host)
                    .arg(port);

    // Создание и запуск клиентов
    QList<ClientLogic *> clients;
    for (int i = 0; i < clientCount; ++i) {
        clients.append(new ClientLogic(host, port, &a));
        clients.last()->start();
    }

    return a.exec();
}
