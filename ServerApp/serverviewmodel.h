#ifndef SERVERVIEWMODEL_H
#define SERVERVIEWMODEL_H

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>

#include "serverworker.h"
#include "iserver.h"
#include "tablemodel.h"

class ServerViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(TableModel* clientTableModel READ clientTableModel CONSTANT)
    Q_PROPERTY(TableModel* dataTableModel READ dataTableModel CONSTANT)
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)


    static constexpr int WORKER_THREAD_WAIT_TIMEOUT_MS  = 5000;     // Таймаут ожидания завершения потока
    static constexpr int MAX_DATA_TABLE_ROWS            = 3000;     // Максимальное количество строк в таблице данных
    static constexpr int DATA_TABLE_TRIM_LENGTH         = 1000;     // Чтобы не обрезать до нуля, оставляем небольшой буфер

public:
    explicit ServerViewModel(QObject *parent = nullptr);
    ~ServerViewModel();

    // Геттеры для QML
    TableModel* clientTableModel() const;
    TableModel* dataTableModel() const;
    QString logText() const;

    // Методы, вызываемые из QML
    Q_INVOKABLE void startServer(AppEnums::ServerType type, quint16 port);
    Q_INVOKABLE void stopServer(AppEnums::ServerType type, quint16 port);
    Q_INVOKABLE void startAllClients();
    Q_INVOKABLE void stopAllClients();
    Q_INVOKABLE void removeDisconnectedClients();
    Q_INVOKABLE void updateClientConfiguration(const QVariantMap& config);
    Q_INVOKABLE void sortClients(int columnIndex);
    Q_INVOKABLE void sortData(int columnIndex);
    Q_INVOKABLE void clearLog();

public slots:
    // Слоты для обработки сигналов от рабочего потока
    void handleClientBatchUpdate(const QList<QVariantMap>& clientBatch);
    void handleDataBatchReceived(const QList<QVariantMap>& dataBatch);
    void handleLogMessage(const QString& message);
    void handleServerStarted();
    void handleServerStopped();

signals:
    void logTextChanged();

    // Сигналы для отправки команд в рабочий поток
    void startServerRequested(AppEnums::ServerType type, quint16 port);
    void stopServerRequested(AppEnums::ServerType type, quint16 port);
    void sendToAllRequested(const QString &command);
    void updateClientConfigRequested(const QVariantMap &config);
    void removeDisconnectedRequested();
    void clearClientsRequested();

private:
    void setupWorkerThread();
    void updateClientInModel(const QVariantMap& clientData);

    // UI модели
    TableModel* m_clientTableModel;
    TableModel* m_dataTableModel;
    QString m_logText;

    // Переменные для хранения порядка сортировки
    Qt::SortOrder m_clientSortOrder;
    Qt::SortOrder m_dataSortOrder;

    // Рабочий поток
    QThread* m_workerThread;
    ServerWorker* m_serverWorker;

};


#endif // SERVERVIEWMODEL_H
