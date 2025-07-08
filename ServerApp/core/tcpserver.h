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

class TcpServer : public IServer {
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();

    int clientCount() const override;
    bool isListening() const override;

public slots:
    void startServer(quint16 port) override;
    void stopServer() override;

    void sendToClient(IClient *client, const QByteArray &data) override;
    void removeClient(IClient *client) override;

private slots:
    void handleNewConnection() override;
    void handleClientDisconnected() override;

    void handleDataReceived(const QByteArray &data) override;

private:
    QTcpServer *m_tcpServer;
    QHash<quintptr, TcpClient *> m_clients;
};

#endif // TCPSERVER_H
