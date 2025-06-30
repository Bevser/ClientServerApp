#include "serverviewmodel.h"
#include <QTime>
#include <QDebug>

// ================= TableModel =================

TableModel::TableModel(const QStringList& keys, const QStringList& headers, QObject *parent)
    : QAbstractTableModel(parent), m_keys(keys), m_headers(headers)
{
    // Устанавливаем ширину колонок по умолчанию (равномерно)
    qreal defaultWidth = 1.0 / keys.size();
    for (int i = 0; i < keys.size(); ++i) {
        m_columnWidths.append(defaultWidth);
    }
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data.size();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_keys.size();
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.size() || index.column() >= m_keys.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const QVariantMap& rowData = m_data.at(index.row());
        const QString& key = m_keys.at(index.column());

        QVariant value = rowData[key];

        // Специальная обработка для различных типов данных
        if (key == Keys::STATUS) {
            int status = value.toInt();
            return AppEnums::statusToString((AppEnums::ClientStatus)status);
        }

        if (key == Keys::ALLOW_SENDING) {
            return value.toBool() ? "Да" : "Нет";
        }

        // Если это payload в данных, преобразуем QVariantMap в строку
        if (key == Keys::PAYLOAD && value.typeId() == QMetaType::QVariantMap) {
            QVariantMap payloadMap = value.toMap();
            QStringList items;
            for (auto it = payloadMap.begin(); it != payloadMap.end(); ++it) {
                items << QString("%1: %2").arg(it.key()).arg(it.value().toString());
            }
            return items.join(", ");
        }

        return value.toString();
    }

    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section < m_headers.size()) {
            return m_headers.at(section);
        }
    }
    return QVariant();
}

void TableModel::setData(const QList<QVariantMap>& data)
{
    beginResetModel();
    m_data = data;
    endResetModel();
}

void TableModel::addRow(const QVariantMap& rowData)
{
    beginInsertRows(QModelIndex(), 0, 0); // Добавляем в начало
    m_data.prepend(rowData);
    endInsertRows();
}

void TableModel::updateRow(int row, const QVariantMap& rowData)
{
    if (row >= 0 && row < m_data.size()) {
        m_data[row] = rowData;
        emit dataChanged(index(row, 0), index(row, m_keys.size() - 1));
    }
}

void TableModel::removeRow(int row)
{
    if (row >= 0 && row < m_data.size()) {
        beginRemoveRows(QModelIndex(), row, row);
        m_data.removeAt(row);
        endRemoveRows();
    }
}

void TableModel::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}

void TableModel::sortByColumn(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= m_keys.size()) return;

    const QString& key = m_keys.at(column);

    beginResetModel();
    std::sort(m_data.begin(), m_data.end(), [key, order](const QVariantMap& a, const QVariantMap& b) {
        QVariant valueA = a[key];
        QVariant valueB = b[key];

        // Специальная обработка для чисел
        if (key == Keys::PORT || key == Keys::STATUS) {
            if (order == Qt::AscendingOrder) {
                return valueA.toInt() < valueB.toInt();
            } else {
                return valueA.toInt() > valueB.toInt();
            }
        }

        // Специальная обработка для времени
        if (key == Keys::TIME_STAMP) {
            QTime timeA = QTime::fromString(valueA.toString(), "hh:mm:ss.zzz");
            QTime timeB = QTime::fromString(valueB.toString(), "hh:mm:ss.zzz");
            if (order == Qt::AscendingOrder) {
                return timeA < timeB;
            } else {
                return timeA > timeB;
            }
        }

        // Обычная строковая сортировка
        if (order == Qt::AscendingOrder) {
            return valueA.toString().toLower() < valueB.toString().toLower();
        } else {
            return valueA.toString().toLower() > valueB.toString().toLower();
        }
    });
    endResetModel();
}

QVariantMap TableModel::getRowData(int row) const
{
    if (row >= 0 && row < m_data.size()) {
        return m_data.at(row);
    }
    return QVariantMap();
}

// ================= ServerViewModel =================

#include <QTime>
#include <QDebug>

ServerViewModel::ServerViewModel(IServer* server, QObject *parent)
    : QObject(parent), m_clientSortOrder(Qt::AscendingOrder), m_dataSortOrder(Qt::AscendingOrder)
{
    // Создаем модели таблиц
    QStringList clientKeys      = {Keys::ID,        Keys::ADDRESS,  Keys::STATUS,   Keys::ALLOW_SENDING};
    QStringList clientHeaders   = {"ID Клиента",    "Адрес",        "Статус",       "Отправка"};
    QList<qreal> clientWidth    = {0.25,            0.30,           0.25,           0.20};

    QStringList dataKeys        = {Keys::TIME_STAMP,Keys::ID,       Keys::TYPE,     Keys::PAYLOAD};
    QStringList dataHeaders     = {"Время",         "ID",           "Тип",          "Сообщение"};
    QList<qreal> dataWidth      = {0.15,            0.20,           0.15,           0.50};


    m_clientTableModel  = new TableModel(clientKeys, clientHeaders, this);
    m_dataTableModel    = new TableModel(dataKeys, dataHeaders, this);

    m_clientTableModel->setColumnWidths(clientWidth);
    m_dataTableModel->setColumnWidths(dataWidth);

    // Настраиваем рабочий поток
    setupWorkerThread();

    // Передаем сервер в рабочий поток
    if (server) {
        server->moveToThread(m_workerThread);
        m_serverWorker->setServer(server);
    }
}

ServerViewModel::~ServerViewModel()
{
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait(WORKER_THREAD_WAIT_TIMEOUT_MS);
        m_workerThread->deleteLater();
    }
}

void ServerViewModel::setupWorkerThread()
{
    m_workerThread = new QThread(this);
    m_serverWorker = new ServerWorker();
    m_serverWorker->moveToThread(m_workerThread);

    // Подключаем сигналы от рабочего потока к UI
    connect(m_serverWorker, &ServerWorker::clientUpdateReady,
            this, &ServerViewModel::handleClientUpdate, Qt::QueuedConnection);
    connect(m_serverWorker, &ServerWorker::dataReceivedReady,
            this, &ServerViewModel::handleDataReceived, Qt::QueuedConnection);
    connect(m_serverWorker, &ServerWorker::logMessageReady,
            this, &ServerViewModel::handleLogMessage, Qt::QueuedConnection);
    connect(m_serverWorker, &ServerWorker::serverStarted,
            this, &ServerViewModel::handleServerStarted, Qt::QueuedConnection);
    connect(m_serverWorker, &ServerWorker::serverStopped,
            this, &ServerViewModel::handleServerStopped, Qt::QueuedConnection);

    // Подключаем сигналы от UI к рабочему потоку
    connect(this, &ServerViewModel::startServerRequested,
            m_serverWorker, &ServerWorker::startServer, Qt::QueuedConnection);
    connect(this, &ServerViewModel::stopServerRequested,
            m_serverWorker, &ServerWorker::stopServer, Qt::QueuedConnection);
    connect(this, &ServerViewModel::sendToAllRequested,
            m_serverWorker, &ServerWorker::sendToAllClients, Qt::QueuedConnection);
    connect(this, &ServerViewModel::updateClientConfigRequested,
            m_serverWorker, &ServerWorker::updateClientConfiguration, Qt::QueuedConnection);
    connect(this, &ServerViewModel::removeDisconnectedRequested,
            m_serverWorker, &ServerWorker::removeDisconnectedClients, Qt::QueuedConnection);
    connect(this, &ServerViewModel::clearClientsRequested,
            m_serverWorker, &ServerWorker::clearClients, Qt::QueuedConnection);

    // Очистка при завершении потока
    connect(m_workerThread, &QThread::finished, m_serverWorker, &QObject::deleteLater);

    m_workerThread->start();
}

void ServerViewModel::startServer(quint16 port)
{
    emit startServerRequested(port);
}

void ServerViewModel::stopServer()
{
    emit stopServerRequested();
}

void ServerViewModel::startAllClients()
{
    QVariantMap message;
    message[Keys::TYPE] = Protocol::MessageType::COMMAND;
    message[Protocol::Keys::COMMAND] = Protocol::Commands::START;
    emit sendToAllRequested(message);
}

void ServerViewModel::stopAllClients()
{
    QVariantMap message;
    message[Keys::TYPE] = Protocol::MessageType::COMMAND;
    message[Protocol::Keys::COMMAND] = Protocol::Commands::STOP;
    emit sendToAllRequested(message);
}

void ServerViewModel::removeDisconnectedClients()
{
    for (int i = m_clientTableModel->rowCount() - 1; i >= 0; --i) {
        QVariantMap client = m_clientTableModel->getRowData(i);
        if (client[Keys::STATUS] == AppEnums::DISCONNECTED)
        {
            m_clientTableModel->removeRow(i);
        }
    }

    emit removeDisconnectedRequested();
}

void ServerViewModel::updateClientConfiguration(const QVariantMap& config)
{
    emit updateClientConfigRequested(config);
}

TableModel* ServerViewModel::clientTableModel() const
{
    return m_clientTableModel;
}

TableModel* ServerViewModel::dataTableModel() const
{
    return m_dataTableModel;
}

QString ServerViewModel::logText() const
{
    return m_logText;
}

void ServerViewModel::sortClients(int columnIndex)
{
    m_clientTableModel->sortByColumn(columnIndex, m_clientSortOrder);
    m_clientSortOrder = (m_clientSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
}

void ServerViewModel::sortData(int columnIndex)
{
    m_dataTableModel->sortByColumn(columnIndex, m_dataSortOrder);
    m_dataSortOrder = (m_dataSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
}

void ServerViewModel::clearLog()
{
    m_logText.clear();
    emit logTextChanged();
}

void ServerViewModel::handleClientUpdate(const QVariantMap& clientData)
{
    updateClientInModel(clientData);
}

void ServerViewModel::handleDataReceived(const QVariantMap& data)
{
    m_dataTableModel->addRow(data);
    if (m_dataTableModel->rowCount() > MAX_DATA_TABLE_ROWS) {
        m_dataTableModel->removeRow(m_dataTableModel->rowCount() - 1);
    }
}

void ServerViewModel::handleLogMessage(const QString& message)
{
    QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
    m_logText.prepend(QString("%1 | %2\n").arg(timestamp).arg(message));

    if (m_logText.length() > MAX_LOG_TEXT_LENGTH) {
        m_logText = m_logText.left(LOG_TRIM_LENGTH);
    }

    emit logTextChanged();
}

void ServerViewModel::handleServerStarted()
{
    handleLogMessage("Сервер успешно запущен");
}

void ServerViewModel::handleServerStopped()
{
    m_clientTableModel->clear();
    m_dataTableModel->clear();
    handleLogMessage("Сервер остановлен");
}

void ServerViewModel::updateClientInModel(const QVariantMap& clientData)
{
    int status = clientData[Keys::STATUS].toInt();
    QString key = (status == AppEnums::DISCONNECTED) ? Keys::ID : Keys::DESCRIPTOR;

    bool found = false;
    for (int i = 0; i < m_clientTableModel->rowCount(); ++i) {
        QVariantMap existingClient = m_clientTableModel->getRowData(i);
        if (existingClient[key].toString() == clientData[key].toString()) {
            m_clientTableModel->updateRow(i, clientData);
            found = true;
            break;
        }
    }

    if (!found && status != AppEnums::DISCONNECTED) {
        m_clientTableModel->addRow(clientData);
    }
}

void ServerViewModel::removeClientFromModel(const QString& descriptor)
{
    for (int i = 0; i < m_clientTableModel->rowCount(); ++i) {
        QVariantMap clientData = m_clientTableModel->getRowData(i);
        if (clientData[Keys::DESCRIPTOR].toString() == descriptor) {
            m_clientTableModel->removeRow(i);
            break;
        }
    }
}

