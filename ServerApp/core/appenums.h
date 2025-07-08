#ifndef APPENUMS_H
#define APPENUMS_H

#include <QObject>
#include <QString>

class AppEnums : public QObject {
    Q_OBJECT

public:
    enum class ServerType { TCP, UDP, MODBUS_TCP, MODBUS_RTU };
    Q_ENUM(ServerType)

    Q_INVOKABLE static QString typeToString(ServerType type) {
        switch (type) {
        case ServerType::TCP:
            return "TCP";
        case ServerType::UDP:
            return "UDP";
        case ServerType::MODBUS_TCP:
            return "MODBUS TCP";
        case ServerType::MODBUS_RTU:
            return "MODBUS RTU";
        default:
            return "Неизвестно";
        }
    }

    enum ServerStatus {
        STOPPED,
        RUNNING,
        ERROR,
    };
    Q_ENUM(ServerStatus)

    Q_INVOKABLE static QString serverStatusToString(ServerStatus status) {
        switch (status) {
        case ServerStatus::STOPPED:
            return "Остановлен";
        case ServerStatus::RUNNING:
            return "Запущен";
        case ServerStatus::ERROR:
            return "Ошибка";
        default:
            return "Неизвестно";
        }
    }

    enum ClientStatus {
        DELETED,      // Удалён
        DISCONNECTED, // Отключен
        AUTHORIZING,  // Подключен, но не прошел авторизацию
        CONNECTED,    // Успешно зарегистрирован
    };
    Q_ENUM(ClientStatus)

    Q_INVOKABLE static QString clientStatusToString(ClientStatus status) {
        switch (status) {
        case DISCONNECTED:
            return "Отключен";
        case AUTHORIZING:
            return "Авторизация";
        case CONNECTED:
            return "Подключен";
        default:
            return "Неизвестно";
        }
    }
};

#endif // APPENUMS_H
