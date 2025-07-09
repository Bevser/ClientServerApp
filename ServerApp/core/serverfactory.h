/**
 * @file serverfactory.h
 * @brief Определяет фабрику для создания объектов серверов.
 */
#ifndef SERVERFACTORY_H
#define SERVERFACTORY_H

#include "appenums.h"
#include "tcpserver.h"

/**
 * @class ServerFactory
 * @brief Класс-фабрика для создания экземпляров серверов различных типов.
 *
 * Предоставляет статический метод для создания сервера в зависимости от
 * переданного типа `AppEnums::ServerType`.
 */
class ServerFactory {
public:
    /**
     * @brief Создает и возвращает экземпляр сервера нужного типа.
     * @param type Тип сервера (TCP, UDP и т.д.).
     * @param parent Родительский объект QObject.
     * @return Указатель на созданный IServer или nullptr, если тип неизвестен.
     */
    static IServer *createServer(AppEnums::ServerType type,
                                 QObject *parent = nullptr) {
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
