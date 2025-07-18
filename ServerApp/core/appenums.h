/**
 * @file appenums.h
 * @brief Определяет перечисления, используемые в приложении.
 */
#ifndef APPENUMS_H
#define APPENUMS_H

#include <QObject>
#include <QString>
#include <QtQmlIntegration>

/**
 * @class AppEnums
 * @brief Класс-обертка для перечислений, используемых в приложении.
 *
 * Предоставляет доступ к перечислениям из C++ в QML, а также
 * статические методы для преобразования значений перечислений в строки.
 */
class AppEnums : public QObject {
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT

public:
    /**
     * @enum ServerType
     * @brief Типы поддерживаемых серверов.
     */
    enum class ServerType { TCP, UDP, MODBUS_TCP, MODBUS_RTU };
    Q_ENUM(ServerType)

    /**
     * @brief Преобразует ServerType в строку.
     * @param type Тип сервера.
     * @return Строковое представление типа.
     */
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

    /**
     * @enum ServerStatus
     * @brief Статусы работы сервера.
     */
    enum ServerStatus {
        STOPPED,
        RUNNING,
        ERROR,
    };
    Q_ENUM(ServerStatus)

    /**
     * @brief Преобразует ServerStatus в строку.
     * @param status Статус сервера.
     * @return Строковое представление статуса.
     */
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

    /**
     * @enum ClientStatus
     * @brief Статусы состояния клиента.
     */
    enum ClientStatus {
        DELETED,      // Удалён
        DISCONNECTED, // Отключен
        AUTHORIZING,  // Подключен, но не прошел авторизацию
        CONNECTED,    // Успешно зарегистрирован
    };
    Q_ENUM(ClientStatus)

    /**
     * @brief Преобразует ClientStatus в строку.
     * @param status Статус клиента.
     * @return Строковое представление статуса.
     */
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
