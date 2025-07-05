#include "serverviewmodel.h"

ServerViewModel::ServerViewModel(QObject *parent)
    : QObject(parent), m_clientSortOrder(Qt::AscendingOrder), m_dataSortOrder(Qt::AscendingOrder) {

    m_clientTableModel  = new ClientTableModel(this);
    m_dataTableModel    = new DataTableModel(this);

    // Настраиваем рабочий поток
    setupWorkerThread();
}

ServerViewModel::~ServerViewModel() {
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait(WORKER_THREAD_WAIT_TIMEOUT_MS);
        m_workerThread->deleteLater();
    }
}

void ServerViewModel::setupWorkerThread() {
    m_workerThread = new QThread(this);
    m_serverWorker = new ServerWorker();
    m_serverWorker->moveToThread(m_workerThread);

    // Подключаем сигналы от рабочего потока к UI
    connect(m_serverWorker, &ServerWorker::clientBatchReady,
            this, &ServerViewModel::handleClientBatchUpdate, Qt::QueuedConnection);
    connect(m_serverWorker, &ServerWorker::dataBatchReady,
            this, &ServerViewModel::handleDataBatchReceived, Qt::QueuedConnection);
    connect(m_serverWorker, &ServerWorker::logBatchReady,
            this, &ServerViewModel::handleLogBatch, Qt::QueuedConnection);
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

void ServerViewModel::startServer(AppEnums::ServerType type, quint16 port) {
    emit startServerRequested(type, port);
}

void ServerViewModel::stopServer(AppEnums::ServerType type, quint16 port) {
    emit stopServerRequested(type, port);
}

void ServerViewModel::startAllClients() {
    emit sendToAllRequested(Protocol::Commands::START);
}

void ServerViewModel::stopAllClients() {
    emit sendToAllRequested(Protocol::Commands::STOP);
}

void ServerViewModel::handleDataBatchReceived(const QList<QVariantMap>& dataBatch) {
    if (m_dataTableModel) {
        m_dataTableModel->addRows(dataBatch);
        if (m_dataTableModel->rowCount() > MAX_DATA_TABLE_ROWS) {
            int rowsToRemove = m_dataTableModel->rowCount() - (MAX_DATA_TABLE_ROWS - DATA_TABLE_TRIM_LENGTH);
            if (rowsToRemove > 0) {
                m_dataTableModel->removeRows(MAX_DATA_TABLE_ROWS - DATA_TABLE_TRIM_LENGTH, rowsToRemove);
            }
        }
    }
}

void ServerViewModel::removeDisconnectedClients() {
    emit removeDisconnectedRequested();
}

void ServerViewModel::updateClientConfiguration(const QVariantMap& config) {
    emit updateClientConfigRequested(config);
}

ClientTableModel* ServerViewModel::clientTableModel() const {
    return m_clientTableModel;
}

DataTableModel* ServerViewModel::dataTableModel() const {
    return m_dataTableModel;
}

QString ServerViewModel::logText() const {
    return m_logText;
}

void ServerViewModel::sortClients(int columnIndex) {
    m_clientTableModel->sortByColumn(columnIndex, m_clientSortOrder);
    m_clientSortOrder = (m_clientSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
}

void ServerViewModel::sortData(int columnIndex) {
    m_dataTableModel->sortByColumn(columnIndex, m_dataSortOrder);
    m_dataSortOrder = (m_dataSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
}

void ServerViewModel::clearLog() {
    m_logText.clear();
    emit logTextChanged();
}

void ServerViewModel::handleLogBatch(const QStringList& logBatch) {
    if (logBatch.isEmpty()) return;
    QStringList reversedLog = logBatch;
    std::reverse(reversedLog.begin(), reversedLog.end());

    m_logText.prepend(reversedLog.join(""));
    emit logTextChanged();
}

void ServerViewModel::handleServerStopped() {
    m_clientTableModel->clear();
    m_dataTableModel->clear();
}

void ServerViewModel::handleClientBatchUpdate(const QList<QVariantMap>& clientBatch) {
    if (clientBatch.isEmpty()) {
        return;
    }

    // 1. Создаём карту обновлений для быстрого доступа по дескриптору
    QHash<QString, QVariantMap> batchMap;
    batchMap.reserve(clientBatch.size());
    for (const QVariantMap& clientData : clientBatch) {
        batchMap.insert(clientData.value(Keys::DESCRIPTOR).toString(), clientData);
    }

    QList<QVariantMap> finalDataList;
    finalDataList.reserve(m_clientTableModel->rowCount());

    // 2. Проходим по текущим данным
    for (int i = 0; i < m_clientTableModel->rowCount(); ++i) {
        QVariantMap existingClient = m_clientTableModel->getRowData(i);
        QString descriptor = existingClient.value(Keys::DESCRIPTOR).toString();

        // Проверяем, есть ли текущий клиент в пакете обновлений
        if (batchMap.contains(descriptor)) {
            // Клиент есть в пакете
            QVariantMap updatedClientData = batchMap.take(descriptor);
            int status = updatedClientData.value(Keys::STATUS).toInt();

            // Если статус не "DELETED", добавляем обновлённые данные в новый список.
            if (status != AppEnums::DELETED) {
                finalDataList.append(updatedClientData);
            }
        } else {
            // Клиента не было в пакете обновлений, сохраняем его без изменений
            finalDataList.append(existingClient);
        }
    }

    // 3. Все клиенты, оставшиеся в batchMap, являются новыми
    for (const QVariantMap& newClientData : batchMap.values()) {
        int status = newClientData.value(Keys::STATUS).toInt();
        // Добавляем нового клиента, только если он не "DISCONNECTED" или "DELETED"
        if (status != AppEnums::DISCONNECTED && status != AppEnums::DELETED) {
            finalDataList.append(newClientData);
        }
    }

    // 4. Атомарно обновляем модель новым списком данных
    m_clientTableModel->setData(finalDataList);
}

