#ifndef ICLIENT_H
#define ICLIENT_H

#include <QObject>

class IClient : public QObject {
    Q_OBJECT

public:
    virtual ~IClient() = default;

    virtual quintptr descriptor() const             = 0;
    virtual QString address() const                 = 0;
    virtual quint16 port() const                    = 0;
    virtual QString id() const                      = 0;
    virtual bool isConnected() const                = 0;

    virtual void setId(const QString& id)           = 0;

    virtual void sendData(const QByteArray& data)   = 0;

    virtual void disconnect()                       = 0;

protected slots:
    virtual void handleConnected()                  = 0;
    virtual void handleDisconnected()               = 0;
    virtual void handleReadyRead()                  = 0;

signals:
    void connected();
    void disconnected();
    void dataReceived(const QByteArray& data);
    void errorOccurred(const QString& message);

protected:
    explicit IClient(QObject* parent = nullptr) : QObject(parent) {}
};

#endif // ICLIENT_H
