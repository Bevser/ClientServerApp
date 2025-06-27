#ifndef ICLIENT_H
#define ICLIENT_H

#include <QObject>

class IClient : public QObject {
public:
    virtual ~IClient() = default;

    virtual QString id() const          = 0;
    virtual bool isConnected() const    = 0;

    virtual void sendData(const QByteArray& data)   = 0;
    virtual QByteArray readData()                   = 0;

    virtual void disconnect()                       = 0;

signals:
    virtual void connected()            = 0;
    virtual void disconnected()         = 0;
    virtual void readyRead()            = 0;
    virtual void errorOccurred()        = 0;

public slots:
    virtual void handleConnected()      = 0;
    virtual void handleDisconnected()   = 0;
    virtual void handleReadyRead()      = 0;
    virtual void handleErrorOccurred()  = 0;

protected:
    explicit IClient(QObject* parent = nullptr) : QObject(parent) {}
};

#endif // ICLIENT_H
