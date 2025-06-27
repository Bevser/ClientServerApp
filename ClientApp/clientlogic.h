#ifndef CLIENTLOGIC_H
#define CLIENTLOGIC_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QRandomGenerator>
#include <QJsonObject>

class ClientLogic : public QObject
{
    Q_OBJECT

public:
    explicit ClientLogic(const QString& host, quint16 port, QObject *parent = nullptr);
    void start();

private slots:
    void connectToServer();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void sendData();
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    void sendJson(const QJsonObject& json);
    QJsonObject generateNetworkMetrics();
    QJsonObject generateDeviceStatus();
    QJsonObject generateLog();
    void startReconnectTimer();

    QString m_host;
    quint16 m_port;
    QTcpSocket* m_socket;
    QTimer* m_reconnectTimer;
    QTimer* m_dataSendTimer;
    QString m_clientId;
    bool m_isStarted;
};
#endif // CLIENTLOGIC_H
