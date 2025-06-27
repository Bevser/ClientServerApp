#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMap>

#include "tcpclient.h"


class TcpServer : public QObject {
    Q_OBJECT

    struct ClientEntry {
        IClient* client;
        bool allowSending = true;
    };

public:
    enum class CommandType {
        Start,
        Stop,
        Configure,
    };

    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();

    int clientCount() const;
    QString commandToString(CommandType command);

signals:
    void clientConnected(quintptr descriptor, QString ip, quint16 port);
    void clientDisconnected(quintptr descriptor, QString ip, quint16 port);
    void dataReceived(quintptr descriptor, const QJsonObject& data, const QString& clientId);
    void logMessage(const QString& message);

public slots:
    void startServer(quint16 port = 8080);
    void stopServer();
    void sendCommandToAll(CommandType command);

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleClientDisconnected();

private:
    QTcpServer* m_tcpServer;
    QList<ClientEntry> m_clients;
};

#endif // TCPSERVER_H
