// serverfactory.h
#ifndef SERVERFACTORY_H
#define SERVERFACTORY_H

#include "tcpserver.h"

enum class ServerType {
    TCP,
    UDP,
    MODBUS_TCP,
    MODBUS_RTU
};

class ServerFactory {
public:
    static IServer* createServer(ServerType type, QObject* parent = nullptr) {
        if (type == ServerType::TCP) {
            return new TcpServer(parent);
        } else if (type == ServerType::UDP) { // Пример
            // return new UdpServer(parent);
            return nullptr;
        }
        // ... и т.д.
        return nullptr; // Неизвестный тип
    }
};

#endif // SERVERFACTORY_H
