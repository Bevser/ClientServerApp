#ifndef SERVERVIEWMODEL_H
#define SERVERVIEWMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include <algorithm>

#include "serverworker.h"
#include "iserver.h"
#include "sharedkeys.h"
#include "appenums.h"

// ================= TableModel Class =================

class TableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TableModel(const QStringList& keys, const QStringList& headers, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Методы для управления данными
    void setData(const QList<QVariantMap>& data);
    void addRow(const QVariantMap& rowData);
    void updateRow(int row, const QVariantMap& rowData);
    void removeRow(int row);
    void clear();
    void sortByColumn(int column, Qt::SortOrder order);

    // Метод для получения данных строки (для QML)
    Q_INVOKABLE QVariantMap getRowData(int row) const;

    // Свойства для получения информации о колонках в QML
    Q_PROPERTY(QStringList columnHeaders READ columnHeaders CONSTANT)
    Q_PROPERTY(QList<qreal> columnWidths READ columnWidths CONSTANT)

    QStringList columnHeaders() const { return m_headers; }
    QList<qreal> columnWidths() const { return m_columnWidths; }

    // Метод для установки ширины колонок
    void setColumnWidths(const QList<qreal>& widths) { m_columnWidths = widths; }

private:
    QStringList m_keys;             // Ключи колонок
    QStringList m_headers;          // Заголовки колонок для отображения
    QList<QVariantMap> m_data;      // Данные таблицы
    QList<qreal> m_columnWidths;    // Ширина колонок (в процентах)
};

// ================= ServerViewModel Class =================

class ServerViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(TableModel* clientTableModel READ clientTableModel CONSTANT)
    Q_PROPERTY(TableModel* dataTableModel READ dataTableModel CONSTANT)
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)


    static constexpr int WORKER_THREAD_WAIT_TIMEOUT_MS  = 5000;     // Таймаут ожидания завершения потока
    static constexpr int MAX_DATA_TABLE_ROWS            = 200;      // Максимальное количество строк в таблице данных
    static constexpr int MAX_LOG_TEXT_LENGTH            = 10000;    // Максимальная длина текста лога и длина после обрезки
    static constexpr int LOG_TRIM_LENGTH                = 8000;     // Чтобы не обрезать до нуля, оставляем небольшой буфер

public:
    explicit ServerViewModel(IServer* server, QObject *parent = nullptr);
    ~ServerViewModel();

    // Геттеры для QML
    TableModel* clientTableModel() const;
    TableModel* dataTableModel() const;
    QString logText() const;

    // Методы, вызываемые из QML
    Q_INVOKABLE void startServer(quint16 port);
    Q_INVOKABLE void stopServer();
    Q_INVOKABLE void startAllClients();
    Q_INVOKABLE void stopAllClients();
    Q_INVOKABLE void removeDisconnectedClients();
    Q_INVOKABLE void updateClientConfiguration(const QVariantMap& config);
    Q_INVOKABLE void sortClients(int columnIndex);
    Q_INVOKABLE void sortData(int columnIndex);
    Q_INVOKABLE void clearLog();

public slots:
    // Слоты для обработки сигналов от рабочего потока
    void handleClientUpdate(const QVariantMap& clientData);
    void handleDataReceived(const QVariantMap& data);
    void handleLogMessage(const QString& message);
    void handleServerStarted();
    void handleServerStopped();

signals:
    void logTextChanged();

    // Сигналы для отправки команд в рабочий поток
    void startServerRequested(quint16 port);
    void stopServerRequested();
    void sendToAllRequested(const QVariantMap &data);
    void updateClientConfigRequested(const QVariantMap &config);
    void removeDisconnectedRequested();
    void clearClientsRequested();

private:
    void setupWorkerThread();
    void updateClientInModel(const QVariantMap& clientData);
    void removeClientFromModel(const QString& descriptor);

    // UI модели (остаются в главном потоке)
    TableModel* m_clientTableModel;
    TableModel* m_dataTableModel;
    QString m_logText;

    // Рабочий поток
    QThread* m_workerThread;
    ServerWorker* m_serverWorker;

    // Переменные для хранения порядка сортировки
    Qt::SortOrder m_clientSortOrder;
    Qt::SortOrder m_dataSortOrder;
};


#endif // SERVERVIEWMODEL_H
