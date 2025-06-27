#ifndef SERVERVIEWMODEL_H
#define SERVERVIEWMODEL_H

#include <QObject>
#include <QList>
#include <QJsonObject>
#include <QThread>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include "tcpserver.h"

// Модель для таблицы клиентов
class ClientTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Roles {
        DisplayRole = Qt::DisplayRole,
        ClientIdRole = Qt::UserRole + 1,
        DescriptorRole,
        IpAddressRole,
        StatusRole
    };

    explicit ClientTableModel(QObject* parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Методы для управления данными
    void addClient(quintptr descriptor, const QString& ip, quint16 port);
    void updateClientId(quintptr descriptor, const QString& clientId);
    void updateClientStatus(quintptr descriptor, const QString& status);
    void removeClient(quintptr descriptor);
    void clear();
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    struct ClientData {
        QString clientId = "Ожидание ID...";
        quintptr descriptor = 0;
        QString ipAddress;
        QString status = "Подключен";
    };

    QList<ClientData> m_clients;
    int m_sortColumn = -1;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
};

// Модель для таблицы данных
class DataTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Roles {
        DisplayRole = Qt::DisplayRole,
        TimestampRole = Qt::UserRole + 1,
        ClientIdRole,
        DataTypeRole,
        ContentRole
    };

    explicit DataTableModel(QObject* parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Методы для управления данными
    void addData(const QString& timestamp, const QString& clientId, const QString& dataType, const QString& content);
    void clear();
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    struct DataEntry {
        QString timestamp;
        QString clientId;
        QString dataType;
        QString content;
    };

    QList<DataEntry> m_data;
    int m_sortColumn = -1;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
    static const int MAX_ENTRIES = 500;
};

// Основной класс ViewModel
class ServerViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ClientTableModel* clientTableModel READ clientTableModel CONSTANT)
    Q_PROPERTY(DataTableModel* dataTableModel READ dataTableModel CONSTANT)
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)

public:
    explicit ServerViewModel(QObject *parent = nullptr);
    ~ServerViewModel();

    ClientTableModel* clientTableModel() const { return m_clientTableModel; }
    DataTableModel* dataTableModel() const { return m_dataTableModel; }
    QString logText() const;

    // Методы, вызываемые из QML
    Q_INVOKABLE void startServer();
    Q_INVOKABLE void stopServer();
    Q_INVOKABLE void startAllClients();
    Q_INVOKABLE void stopAllClients();
    Q_INVOKABLE void sortClients(int column);
    Q_INVOKABLE void sortData(int column);

signals:
    void logTextChanged();

private slots:
    // Слоты для получения сигналов от ServerLogic
    void onClientConnected(quintptr descriptor, const QString& ip, quint16 port);
    void onClientDisconnected(quintptr descriptor);
    void onDataReceived(quintptr descriptor, const QJsonObject& data, const QString& clientId);
    void onLogMessage(const QString& message);

private:
    TcpServer* m_serverLogic;
    QThread* m_serverThread;

    ClientTableModel* m_clientTableModel;
    DataTableModel* m_dataTableModel;
    QString m_logText;

    // Для отслеживания порядка сортировки
    int m_clientSortColumn = -1;
    Qt::SortOrder m_clientSortOrder = Qt::AscendingOrder;
    int m_dataSortColumn = -1;
    Qt::SortOrder m_dataSortOrder = Qt::AscendingOrder;
};

#endif // SERVERVIEWMODEL_H
