#ifndef DATAPROCESSING_H
#define DATAPROCESSING_H

#include <QObject>
#include <QJsonObject>
#include <QHash>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

#include "iclient.h"
#include "iserver.h"
#include "sharedkeys.h"
#include "appenums.h"

class DataProcessing : public QObject
{
    Q_OBJECT

    struct ClientState {
        IClient* client                 = nullptr;
        AppEnums::ClientStatus status   = AppEnums::DISCONNECTED;
        bool allowSending               = false;
        QVariantMap configuration;
    };

public:
    explicit DataProcessing(QObject *parent = nullptr);
    ~DataProcessing();

    void addServer(IServer* server);

public slots:
    void sendToAll(const QVariantMap &data);

    void dataToClient(const QVariantMap &data);
    void routeDataToClient(IClient* client, const QByteArray& data);
    void removeDisconnectedClients();
    void clearClients();

private slots:
    void handleClientConnected(IClient* client);
    void handleClientDisconnected(IClient* client);
    void handleDataReceived(IClient* client, const QByteArray &data);


signals:
    void clientUpdate(const QVariantMap &data);
    void dataReceived(const QVariantMap &data);
    void logMessage(const QString& message);


private:

    void parseJsonData(IClient* client, const QByteArray &data);
    void registerClient(IClient* client, const QString &id, const QJsonObject &payload);
    QVariantMap getClientDataMap(const ClientState& state);

    QList<IServer*> m_servers;
    QHash<quintptr, ClientState> m_clients;
};

#endif // DATAPROCESSING_H
