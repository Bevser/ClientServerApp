#include "serverworker.h"

ServerWorker::ServerWorker(QObject *parent)
    : QObject(parent), m_dataProcessing(nullptr) {

    // Создаем DataProcessing в рабочем потоке
    m_dataProcessing = new DataProcessing(this);

    // Подключаем сигналы для передачи в UI поток
    connect(m_dataProcessing, &DataProcessing::logMessage,
            this, &ServerWorker::handleLogMessage);

    m_batchTimer = new QTimer(this);
    connect(m_batchTimer, &QTimer::timeout, this, &ServerWorker::handleBatchTimerTimeout);
}

ServerWorker::~ServerWorker() {
}

void ServerWorker::handleBatchTimerTimeout() {
    if (m_dataProcessing) {
        // Забираем пакет данных
        QList<QVariantMap> dataBatch = m_dataProcessing->takeDataBatch();
        if (!dataBatch.isEmpty()) {
            emit dataBatchReady(dataBatch);
        }

        // Забираем пакет обновлений клиентов
        QList<QVariantMap> clientBatch = m_dataProcessing->takeClientUpdatesBatch();
        if (!clientBatch.isEmpty()) {
            m_batchTimer->setInterval(BATCH_TIMEOUT_SLOW_MODE_MS);
            emit clientBatchReady(clientBatch);
        }
        else
        {
            m_batchTimer->setInterval(BATCH_TIMEOUT_MS);
        }

        // Забираем логи
        if (!m_logBatch.isEmpty()) {
            emit logBatchReady(m_logBatch);
            m_logBatch.clear();
        }
    }
}

void ServerWorker::handleLogMessage(const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss");
    m_logBatch.append(QString("[%1] %2\n").arg(timestamp).arg(message));
}

void ServerWorker::startServer(AppEnums::ServerType type, quint16 port) {
    IServer* server = ServerFactory::createServer(type);
    if (!server) {
        handleLogMessage("Ошибка: не удалось создать сервер.");
        return;
    }

    m_dataProcessing->addServer(server);
    m_servers[port] = server;
    server->startServer(port);
    m_batchTimer->start(BATCH_TIMEOUT_MS);
    handleLogMessage("Сервер успешно запущен.");
    emit serverStarted();
}

void ServerWorker::stopServer(AppEnums::ServerType type, quint16 port) {
    IServer* server = m_servers[port];

    if (server) {
        server->stopServer();
        m_dataProcessing->clearClients();

        m_servers.remove(port);
        delete server;
        server = nullptr;
        m_batchTimer->stop();

        handleLogMessage("Сервер остановлен.");
        emit serverStopped();
    }
}

void ServerWorker::sendToAllClients(const QString &data) {
    if (m_dataProcessing) {
        m_dataProcessing->sendDataToAll(data);
    }
}

void ServerWorker::updateClientConfiguration(const QVariantMap &config) {
    if (m_dataProcessing) {
        m_dataProcessing->routeDataToClient(config);
    }
}

void ServerWorker::removeDisconnectedClients() {
    if (m_dataProcessing) {
        m_dataProcessing->removeDisconnectedClients();
    }
}

void ServerWorker::clearClients() {
    if (m_dataProcessing) {
        m_dataProcessing->clearClients();
    }
}
