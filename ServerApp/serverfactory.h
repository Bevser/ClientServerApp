// serverfactory.h
#ifndef SERVERFACTORY_H
#define SERVERFACTORY_H

#include "tcpserver.h"
#include "appenums.h"

class ServerFactory {
public:
    static IServer* createServer(AppEnums::ServerType type, QObject* parent = nullptr) {
        if (type == AppEnums::ServerType::TCP) {
            return new TcpServer(parent);
        } else if (type == AppEnums::ServerType::UDP) { // Пример
            // return new UdpServer(parent);
            return nullptr;
        }
        // ... и т.д.
        return nullptr; // Неизвестный тип
    }
};

#endif // SERVERFACTORY_H
