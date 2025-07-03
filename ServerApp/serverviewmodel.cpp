#include "serverviewmodel.h"

ServerViewModel::ServerViewModel(QObject *parent)
    : QObject(parent), m_clientSortOrder(Qt::AscendingOrder), m_dataSortOrder(Qt::AscendingOrder) {
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
    }
}

void ServerViewModel::removeDisconnectedClients() {
    emit removeDisconnectedRequested();
}

void ServerViewModel::updateClientConfiguration(const QVariantMap& config) {
    emit updateClientConfigRequested(config);
}

TableModel* ServerViewModel::clientTableModel() const {
    return m_clientTableModel;
}

TableModel* ServerViewModel::dataTableModel() const {
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

void ServerViewModel::handleLogMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss");
    m_logText.prepend(QString("[%1] %2\n").arg(timestamp).arg(message));

    emit logTextChanged();
}

void ServerViewModel::handleServerStarted() {
    handleLogMessage("Сервер успешно запущен");
}

void ServerViewModel::handleServerStopped() {
    m_clientTableModel->clear();
    m_dataTableModel->clear();
    handleLogMessage("Сервер остановлен");
}

void ServerViewModel::handleClientBatchUpdate(const QList<QVariantMap>& clientBatch) {
    for (const QVariantMap& clientData : clientBatch) {
        updateClientInModel(clientData);
    }
}

void ServerViewModel::updateClientInModel(const QVariantMap& clientData) {
    int status = clientData[Keys::STATUS].toInt();

    bool found = false;
    for (int i = 0; i < m_clientTableModel->rowCount(); ++i) {
        QVariantMap existingClient = m_clientTableModel->getRowData(i);
        if (existingClient[Keys::DESCRIPTOR].toString() == clientData[Keys::DESCRIPTOR].toString()) {
            if (status == AppEnums::DELETED) {
                m_clientTableModel->removeRow(i);
            } else {
                m_clientTableModel->updateRow(i, clientData);
            }
            return;
        }
    }

    if (status != AppEnums::DISCONNECTED) {
        m_clientTableModel->addRow(clientData);
    }
}

