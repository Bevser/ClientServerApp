#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include "iclient.h"

class TcpClient : public IClient {
    Q_OBJECT

    static constexpr char DEFAULT_ID[] = "Ожидание Id...";

public:

    explicit TcpClient(QTcpSocket *socket, QObject *parent = nullptr);

    // *** ГЕТТЕРЫ ***

    QString id() const override;
    bool isConnected() const override;

    quintptr descriptor() const;
    QString peerAddress() const;
    quint16 peerPort() const;
    QAbstractSocket::SocketError error() const;

    // *** СЕТТОРЫ ***

    void id(const QString& id);

    // *** МЕТОДЫ ***

    void disconnect() override;

    void sendData(const QJsonObject& json);

    void sendData(const QByteArray& data) override;
    QByteArray readData() override;

signals:
    void connected() override;
    void disconnected() override;
    void readyRead() override;
    void errorOccurred() override;

private slots:
    void connectToServer(const QString &host, quint16 port);

    void handleConnected() override;
    void handleDisconnected() override;
    void handleReadyRead() override;
    void handleErrorOccurred() override;

private:
    QTcpSocket* m_socket;
    QString m_id;
};

#endif // TCPCLIENT_H
