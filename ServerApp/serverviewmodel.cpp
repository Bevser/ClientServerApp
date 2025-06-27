#include "serverviewmodel.h"
#include <QDateTime>
#include <QJsonValue>
#include <algorithm>

// ClientTableModel Implementation
ClientTableModel::ClientTableModel(QObject* parent) : QAbstractTableModel(parent) {}

int ClientTableModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return m_clients.size();
}

int ClientTableModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return 3; // ID, IP, Status
}

QVariant ClientTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_clients.size() || index.column() >= 3)
        return QVariant();

    const ClientData& client = m_clients.at(index.row());

    // Главная роль для отображения - всегда возвращаем строку
    if (role == Qt::DisplayRole || role == DisplayRole) {
        switch (index.column()) {
        case 0: return client.clientId;
        case 1: return client.ipAddress;
        case 2: return client.status;
        default: return QVariant();
        }
    }

    // Дополнительные роли для QML
    switch (role) {
    case ClientIdRole: return client.clientId;
    case DescriptorRole: return static_cast<qulonglong>(client.descriptor);
    case IpAddressRole: return client.ipAddress;
    case StatusRole: return client.status;
    default: return QVariant();
    }
}

QVariant ClientTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return "ID Клиента";
        case 1: return "IP Адрес";
        case 2: return "Статус";
        default: return QVariant();
        }
    }
    return QVariant();
}

QHash<int, QByteArray> ClientTableModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[ClientIdRole] = "clientId";
    roles[DescriptorRole] = "descriptor";
    roles[IpAddressRole] = "ipAddress";
    roles[StatusRole] = "status";
    return roles;
}

void ClientTableModel::addClient(quintptr descriptor, const QString& ip, quint16 port) {
    beginInsertRows(QModelIndex(), m_clients.size(), m_clients.size());
    ClientData client;
    client.descriptor = descriptor;
    client.ipAddress = QString("%1:%2").arg(ip).arg(port);
    client.status = "Подключен";
    client.clientId = "Ожидание ID...";
    m_clients.append(client);
    endInsertRows();
}

void ClientTableModel::updateClientId(quintptr descriptor, const QString& clientId) {
    for (int i = 0; i < m_clients.size(); ++i) {
        if (m_clients[i].descriptor == descriptor) {
            if (m_clients[i].clientId != clientId) {
                m_clients[i].clientId = clientId;
                QModelIndex topLeft = createIndex(i, 0);
                QModelIndex bottomRight = createIndex(i, 0);
                emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole});
            }
            break;
        }
    }
}

void ClientTableModel::updateClientStatus(quintptr descriptor, const QString& status) {
    for (int i = 0; i < m_clients.size(); ++i) {
        if (m_clients[i].descriptor == descriptor) {
            if (m_clients[i].status != status) {
                m_clients[i].status = status;
                QModelIndex topLeft = createIndex(i, 2);
                QModelIndex bottomRight = createIndex(i, 2);
                emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole});
            }
            break;
        }
    }
}

void ClientTableModel::removeClient(quintptr descriptor) {
    for (int i = 0; i < m_clients.size(); ++i) {
        if (m_clients[i].descriptor == descriptor) {
            beginRemoveRows(QModelIndex(), i, i);
            m_clients.removeAt(i);
            endRemoveRows();
            break;
        }
    }
}

void ClientTableModel::clear() {
    if (!m_clients.isEmpty()) {
        beginResetModel();
        m_clients.clear();
        endResetModel();
    }
}

void ClientTableModel::sort(int column, Qt::SortOrder order) {
    if (column < 0 || column >= columnCount() || m_clients.isEmpty())
        return;

    m_sortColumn = column;
    m_sortOrder = order;

    beginResetModel();
    std::sort(m_clients.begin(), m_clients.end(), [column, order](const ClientData& a, const ClientData& b) {
        QString valueA, valueB;
        switch (column) {
        case 0: valueA = a.clientId; valueB = b.clientId; break;
        case 1: valueA = a.ipAddress; valueB = b.ipAddress; break;
        case 2: valueA = a.status; valueB = b.status; break;
        default: return false;
        }
        return order == Qt::AscendingOrder ? valueA < valueB : valueA > valueB;
    });
    endResetModel();
}

// DataTableModel Implementation
DataTableModel::DataTableModel(QObject* parent) : QAbstractTableModel(parent) {}

int DataTableModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return m_data.size();
}

int DataTableModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return 4; // Timestamp, ClientId, DataType, Content
}

QVariant DataTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_data.size() || index.column() >= 4)
        return QVariant();

    const DataEntry& entry = m_data.at(index.row());

    // Главная роль для отображения - всегда возвращаем строку
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return entry.timestamp;
        case 1: return entry.clientId;
        case 2: return entry.dataType;
        case 3: return entry.content;
        default: return QVariant();
        }
    }

    // Дополнительные роли для QML
    switch (role) {
    case TimestampRole: return entry.timestamp;
    case ClientIdRole: return entry.clientId;
    case DataTypeRole: return entry.dataType;
    case ContentRole: return entry.content;
    default: return QVariant();
    }
}

QVariant DataTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return "Время";
        case 1: return "ID Клиента";
        case 2: return "Тип";
        case 3: return "Содержимое";
        default: return QVariant();
        }
    }
    return QVariant();
}

QHash<int, QByteArray> DataTableModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[TimestampRole] = "timestamp";
    roles[ClientIdRole] = "clientId";
    roles[DataTypeRole] = "dataType";
    roles[ContentRole] = "content";
    return roles;
}

void DataTableModel::addData(const QString& timestamp, const QString& clientId, const QString& dataType, const QString& content) {
    // Удаляем старые записи если превышен лимит
    while (m_data.size() >= MAX_ENTRIES) {
        beginRemoveRows(QModelIndex(), m_data.size() - 1, m_data.size() - 1);
        m_data.removeLast();
        endRemoveRows();
    }

    beginInsertRows(QModelIndex(), 0, 0);
    DataEntry entry;
    entry.timestamp = timestamp;
    entry.clientId = clientId;
    entry.dataType = dataType;
    entry.content = content;
    m_data.prepend(entry); // Добавляем в начало для показа последних данных сверху
    endInsertRows();
}

void DataTableModel::clear() {
    if (!m_data.isEmpty()) {
        beginResetModel();
        m_data.clear();
        endResetModel();
    }
}

void DataTableModel::sort(int column, Qt::SortOrder order) {
    if (column < 0 || column >= columnCount() || m_data.isEmpty())
        return;

    m_sortColumn = column;
    m_sortOrder = order;

    beginResetModel();
    std::sort(m_data.begin(), m_data.end(), [column, order](const DataEntry& a, const DataEntry& b) {
        QString valueA, valueB;
        switch (column) {
        case 0: valueA = a.timestamp; valueB = b.timestamp; break;
        case 1: valueA = a.clientId; valueB = b.clientId; break;
        case 2: valueA = a.dataType; valueB = b.dataType; break;
        case 3: valueA = a.content; valueB = b.content; break;
        default: return false;
        }
        return order == Qt::AscendingOrder ? valueA < valueB : valueA > valueB;
    });
    endResetModel();
}

// ServerViewModel Implementation
ServerViewModel::ServerViewModel(QObject *parent)
    : QObject(parent)
    , m_serverLogic(new TcpServer)
    , m_serverThread(new QThread(this))
    , m_clientTableModel(new ClientTableModel(this))
    , m_dataTableModel(new DataTableModel(this))
{
    m_serverLogic->moveToThread(m_serverThread);
    connect(m_serverThread, &QThread::finished, m_serverLogic, &QObject::deleteLater);
    connect(m_serverLogic, &TcpServer::clientConnected, this, &ServerViewModel::onClientConnected);
    connect(m_serverLogic, &TcpServer::clientDisconnected, this, &ServerViewModel::onClientDisconnected);
    connect(m_serverLogic, &TcpServer::dataReceived, this, &ServerViewModel::onDataReceived);
    connect(m_serverLogic, &TcpServer::logMessage, this, &ServerViewModel::onLogMessage);
    m_serverThread->start();
}

ServerViewModel::~ServerViewModel() {
    m_serverThread->quit();
    m_serverThread->wait();
}

QString ServerViewModel::logText() const {
    return m_logText;
}

void ServerViewModel::startServer() {
    QMetaObject::invokeMethod(m_serverLogic, "startServer", Qt::QueuedConnection, Q_ARG(quint16, 12345));
}

void ServerViewModel::stopServer() {
    QMetaObject::invokeMethod(m_serverLogic, "stopServer", Qt::QueuedConnection);
    m_clientTableModel->clear();
    m_dataTableModel->clear();
}

void ServerViewModel::startAllClients() {
    QMetaObject::invokeMethod(m_serverLogic, "sendCommandToAll", Qt::QueuedConnection, Q_ARG(TcpServer::CommandType, TcpServer::CommandType::Start));
}

void ServerViewModel::stopAllClients() {
    QMetaObject::invokeMethod(m_serverLogic, "sendCommandToAll", Qt::QueuedConnection, Q_ARG(TcpServer::CommandType, TcpServer::CommandType::Stop));
}

void ServerViewModel::sortClients(int column) {
    if (m_clientSortColumn == column) {
        // Меняем порядок сортировки если колонка та же
        m_clientSortOrder = (m_clientSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        // Новая колонка - начинаем с возрастающего порядка
        m_clientSortColumn = column;
        m_clientSortOrder = Qt::AscendingOrder;
    }
    m_clientTableModel->sort(column, m_clientSortOrder);
}

void ServerViewModel::sortData(int column) {
    if (m_dataSortColumn == column) {
        // Меняем порядок сортировки если колонка та же
        m_dataSortOrder = (m_dataSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        // Новая колонка - начинаем с возрастающего порядка
        m_dataSortColumn = column;
        m_dataSortOrder = Qt::AscendingOrder;
    }
    m_dataTableModel->sort(column, m_dataSortOrder);
}

void ServerViewModel::onClientConnected(quintptr descriptor, const QString& ip, quint16 port) {
    m_clientTableModel->addClient(descriptor, ip, port);
}

void ServerViewModel::onClientDisconnected(quintptr descriptor) {
    m_clientTableModel->updateClientStatus(descriptor, "Отключен");
}

void ServerViewModel::onDataReceived(quintptr descriptor, const QJsonObject& data, const QString& clientId) {
    m_clientTableModel->updateClientId(descriptor, clientId);

    QString type = data["type"].toString("Unknown");
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    if (type == "NetworkMetrics" || type == "DeviceStatus") {
        for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
            if (it.key() == "type" || it.key() == "id") continue;
            QString content = QString("%1: %2").arg(it.key(), it.value().toVariant().toString());
            m_dataTableModel->addData(timestamp, clientId, type, content);
        }
    } else if (type != "Identification") {
        QString content;
        for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
            if (it.key() == "type" || it.key() == "id") continue;
            content += it.key() + ": " + it.value().toVariant().toString() + "; ";
        }
        if (!content.isEmpty()) {
            content.chop(2); // Убираем последние "; "
        }
        m_dataTableModel->addData(timestamp, clientId, type, content);
    }
}

void ServerViewModel::onLogMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    m_logText.prepend(QString("[%1] %2\n").arg(timestamp, message));
    emit logTextChanged();
}

#include "serverviewmodel.moc"
