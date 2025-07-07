#ifndef SERVERWORKER_H
#define SERVERWORKER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include "dataprocessing.h"
#include "iserver.h"
#include "serverfactory.h"

class ServerWorker : public QObject
{
    Q_OBJECT

    static constexpr int BATCH_TIMEOUT_MS           = 500;
    static constexpr int BATCH_TIMEOUT_SLOW_MODE_MS = 2000;

public:
    explicit ServerWorker(QObject *parent = nullptr);
    ~ServerWorker();

    DataProcessing* dataProcessing() const { return m_dataProcessing; }

public slots:
    void startServer(AppEnums::ServerType type, quint16 port);
    void stopServer(AppEnums::ServerType type, quint16 port);
    void deleteServer(AppEnums::ServerType type, quint16 port);
    void sendToAllClients(const QString &data);
    void updateClientConfiguration(const QVariantMap &config);
    void removeDisconnectedClients();
    void clearClients();

signals:
    // Сигналы для передачи в UI поток
    void serverStarted();
    void serverStopped();
    void serverStatusUpdate(AppEnums::ServerType type, quint16 port, AppEnums::ServerStatus status, int connections);
    void clientBatchReady(const QList<QVariantMap> &clientBatch);
    void dataBatchReady(const QList<QVariantMap> &dataBatch);
    void logBatchReady(const QStringList &logBatch);

private slots:
    void handleLogMessage(const QString &message);
    void handleBatchTimerTimeout();

private:
    QTimer* m_batchTimer;
    QStringList m_logBatch;

    DataProcessing* m_dataProcessing;
    QHash<QPair<AppEnums::ServerType, quint16>, IServer*> m_servers;
};

#endif // SERVERWORKER_H
