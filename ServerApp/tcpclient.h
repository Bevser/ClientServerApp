#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include "iclient.h"

class TcpClient : public IClient {
    Q_OBJECT

public:
    explicit TcpClient(QTcpSocket *socket, QObject *parent = nullptr);

    QString id() const override;
    bool isConnected() const override;

    quintptr descriptor() const override;
    QString address() const override;
    quint16 port() const override;

    void setId(const QString& id) override;

    void connectToHost(const QString &host, quint16 port);
    void disconnect() override;

    void sendData(const QJsonObject& json);
    void sendData(const QByteArray& data) override;

private slots:
    void handleConnected() override;
    void handleDisconnected() override;
    void handleReadyRead() override;
    void handleError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket* m_socket;
    QString m_id;
};

#endif // TCPCLIENT_H
