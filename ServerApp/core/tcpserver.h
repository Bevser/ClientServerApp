/**
 * @file tcpserver.h
 * @brief Определяет класс TcpServer, реализующий IServer для TCP-протокола.
 */
#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMap>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "../common/tcpclient.h"
#include "core/iserver.h"

/**
 * @class TcpServer
 * @brief Реализация интерфейса IServer для TCP-сервера.
 *
 * Управляет QTcpServer и обрабатывает TCP-подключения, создавая
 * для каждого из них объект TcpClient.
 */
class TcpServer : public IServer {
    Q_OBJECT

public:
    /**
     * @brief Конструктор класса TcpServer.
     * @param parent Родительский объект QObject.
     */
    explicit TcpServer(QObject *parent = nullptr);
    /**
     * @brief Деструктор класса TcpServer.
     */
    ~TcpServer();

    /**
     * @brief Возвращает количество подключенных TCP-клиентов.
     * @return Число клиентов.
     */
    int clientCount() const override;
    /**
     * @brief Проверяет, прослушивает ли TCP-сервер порт.
     * @return true, если сервер активен, иначе false.
     */
    bool isListening() const override;

public slots:
    /**
     * @brief Запускает TCP-сервер на указанном порту.
     * @param port Номер порта.
     */
    void startServer(quint16 port) override;
    /**
     * @brief Останавливает TCP-сервер и отключает всех клиентов.
     */
    void stopServer() override;

    /**
     * @brief Отправляет данные указанному TCP-клиенту.
     * @param client Указатель на клиента (должен быть TcpClient).
     * @param data Данные для отправки.
     */
    void sendToClient(IClient *client, const QByteArray &data) override;
    /**
     * @brief Удаляет TCP-клиента с сервера.
     * @param client Указатель на клиента для удаления.
     */
    void removeClient(IClient *client) override;

private slots:
    /**
     * @brief Обрабатывает новое входящее TCP-подключение.
     */
    void handleNewConnection() override;
    /**
     * @brief Обрабатывает сигнал отключения от TCP-клиента.
     */
    void handleClientDisconnected() override;
    /**
     * @brief Обрабатывает данные, полученные от TCP-клиента.
     * @param data Полученные данные.
     */
    void handleDataReceived(const QByteArray &data) override;

private:
    /// @brief Указатель на основной объект QTcpServer.
    QTcpServer *m_tcpServer;
    /// @brief Хеш-таблица для хранения подключенных клиентов по их дескрипторам.
    QHash<quintptr, TcpClient *> m_clients;
};

#endif // TCPSERVER_H
