/**
 * @file iserver.h
 * @brief Определяет интерфейс IServer для всех типов серверов.
 */
#ifndef ISERVER_H
#define ISERVER_H

#include <QObject>

#include "../common/iclient.h"

/**
 * @class IServer
 * @brief Абстрактный базовый класс (интерфейс) для серверных реализаций.
 *
 * Определяет общий набор функций и сигналов, которые должен предоставлять каждый класс сервера,
 * независимо от используемого протокола (TCP, UDP и т.д.).
 */
class IServer : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Конструктор интерфейса IServer.
     * @param parent Родительский объект QObject.
     */
    explicit IServer(QObject *parent = nullptr) : QObject(parent) {}
    /**
     * @brief Виртуальный деструктор по умолчанию.
     */
    virtual ~IServer() = default;

    /**
     * @brief Возвращает количество подключенных клиентов.
     * @return Число клиентов.
     */
    virtual int clientCount() const = 0;
    /**
     * @brief Проверяет, активен ли сервер (прослушивает ли порт).
     * @return true, если сервер активен, иначе false.
     */
    virtual bool isListening() const = 0;

public slots:
    /**
     * @brief Запускает сервер на указанном порту.
     * @param port Номер порта.
     */
    virtual void startServer(quint16 port) = 0;
    /**
     * @brief Останавливает сервер.
     */
    virtual void stopServer() = 0;

    /**
     * @brief Отправляет данные указанному клиенту.
     * @param client Указатель на клиента.
     * @param data Данные для отправки.
     */
    virtual void sendToClient(IClient *client, const QByteArray &data) = 0;
    /**
     * @brief Удаляет клиента с сервера.
     * @param client Указатель на клиента, которого нужно удалить.
     */
    virtual void removeClient(IClient *client) = 0;

protected slots:
    /**
     * @brief Внутренний слот для обработки нового подключения.
     */
    virtual void handleNewConnection() = 0;
    /**
     * @brief Внутренний слот для обработки отключения клиента.
     */
    virtual void handleClientDisconnected() = 0;
    /**
     * @brief Внутренний слот для обработки полученных данных.
     * @param data Полученные данные.
     */
    virtual void handleDataReceived(const QByteArray &data) = 0;

signals:
    /**
     * @brief Сигнал, испускаемый при подключении нового клиента.
     * @param client Указатель на подключенного клиента.
     */
    void clientConnected(IClient *client);
    /**
     * @brief Сигнал, испускаемый при отключении клиента.
     * @param client Указатель на отключившегося клиента.
     */
    void clientDisconnected(IClient *client);
    /**
     * @brief Сигнал, испускаемый при получении данных от клиента.
     * @param client Указатель на клиента-отправителя.
     * @param data Полученные данные.
     */
    void dataReceived(IClient *client, const QByteArray &data);
    /**
     * @brief Сигнал для отправки лог-сообщения.
     * @param message Текст сообщения.
     */
    void logMessage(const QString &message);
};

#endif // ISERVER_H
