#ifndef APPENUMS_H
#define APPENUMS_H

#include <QObject>
#include <QString>

class AppEnums : public QObject {
    Q_OBJECT

public:

    enum class ServerType {
        TCP,
        UDP,
        MODBUS_TCP,
        MODBUS_RTU
    };
    Q_ENUM(ServerType)

    enum ClientStatus {
        DELETED,        // Удалён
        DISCONNECTED,   // Отключен
        AUTHORIZING,    // Подключен, но не прошел авторизацию
        CONNECTED,      // Успешно зарегистрирован
    };
    Q_ENUM(ClientStatus)

    Q_INVOKABLE static QString statusToString(ClientStatus status) {
        switch (status) {
        case DISCONNECTED:  return "Отключен";
        case AUTHORIZING:   return "Авторизация";
        case CONNECTED:     return "Подключен";
        default:            return "Неизвестно";
        }
    }
};

#endif // APPENUMS_H
