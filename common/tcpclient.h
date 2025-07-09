/**
 * @file tcpclient.h
 * @brief Определяет класс TcpClient, реализующий IClient для TCP-протокола.
 */
#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "iclient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTcpSocket>

/**
 * @class TcpClient
 * @brief Реализация интерфейса IClient для TCP-клиента.
 *
 * Этот класс является оберткой над QTcpSocket, предоставляя
 * функциональность, определенную в интерфейсе IClient.
 */
class TcpClient : public IClient {
    Q_OBJECT

public:

    /**
     * @brief Конструктор класса TcpClient.
     * @param socket Указатель на существующий QTcpSocket.
     * @param parent Родительский объект QObject.
     */
    explicit TcpClient(QTcpSocket *socket, QObject *parent = nullptr);

    /**
     * @brief Возвращает ID клиента.
     */
    QString id() const override;
    /**
     * @brief Проверяет, подключен ли сокет.
     */
    bool isConnected() const override;

    /**
     * @brief Возвращает дескриптор сокета.
     */
    quintptr descriptor() const override;
    /**
     * @brief Возвращает IP-адрес удаленной стороны.
     */
    QString address() const override;
    /**
     * @brief Возвращает порт удаленной стороны.
     */
    quint16 port() const override;

    /**
     * @brief Устанавливает ID клиента.
     */
    void setId(const QString &id) override;

    /**
     * @brief Подключается к хосту.
     */
    void connectToHost(const QString &host, quint16 port) override;
    /**
     * @brief Отключается от хоста.
     */
    void disconnect() override;

    /**
     * @brief Отправляет данные в виде JSON-объекта.
     * @param json JSON-объект для отправки.
     */
    void sendData(const QJsonObject &json);
    /**
     * @brief Отправляет данные в виде массива байт.
     * @param data Данные для отправки.
     */
    void sendData(const QByteArray &data) override;

private slots:
    /**
     * @brief Обрабатывает сигнал connected от сокета.
     */
    void handleConnected() override;
    /**
     * @brief Обрабатывает сигнал disconnected от сокета.
     */
    void handleDisconnected() override;
    /**
     * @brief Обрабатывает сигнал readyRead от сокета.
     */
    void handleReadyRead() override;
    /**
     * @brief Обрабатывает сигнал errorOccurred от сокета.
     * @param socketError Код ошибки сокета.
     */
    void handleError(QAbstractSocket::SocketError socketError);

private:
    /// @brief Указатель на QTcpSocket.
    QTcpSocket *m_socket;
    /// @brief Строковый идентификатор клиента.
    QString m_id;
    /// @brief Дескриптор сокета.
    quintptr m_descriptor;
};

#endif // TCPCLIENT_H
