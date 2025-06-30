#ifndef ISERVER_H
#define ISERVER_H

#include <QObject>
#include "iclient.h"

class IServer : public QObject {
    Q_OBJECT

public:
    explicit IServer(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IServer() = default;

    virtual int clientCount() const             = 0;

public slots:
    virtual void startServer(quint16 port)      = 0;
    virtual void stopServer()                   = 0;

    virtual void sendToClient(IClient* client, const QByteArray &data) = 0;
    virtual void removeClient(IClient* client)  = 0;

protected slots:
    virtual void handleNewConnection()          = 0;
    virtual void handleClientDisconnected()     = 0;

    virtual void handleDataReceived(const QByteArray &data) = 0;

signals:
    void clientConnected(IClient* client);
    void clientDisconnected(IClient* client);
    void dataReceived(IClient* client, const QByteArray &data);
    void logMessage(const QString& message);
};

#endif // ISERVER_H
