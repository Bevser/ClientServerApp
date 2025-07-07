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
        } else { // Если нагрузка большая, то уменьшаем частоту обновления
            m_batchTimer->setInterval(BATCH_TIMEOUT_MS);
        }

        // Забираем логи
        if (!m_logBatch.isEmpty()) {
            emit logBatchReady(m_logBatch);
            m_logBatch.clear();
        }

        // Обновляем статусы серверов
        for (auto it = m_servers.constBegin(); it != m_servers.constEnd(); ++it) {
            if (it.value()->isListening()) {
                const auto& key = it.key();
                emit serverStatusUpdate(key.first, key.second,
                                        AppEnums::ServerStatus::RUNNING, it.value()->clientCount());
            }
        }
    }
}

void ServerWorker::handleLogMessage(const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss");
    m_logBatch.append(QString("[%1] %2\n").arg(timestamp).arg(message));
}

void ServerWorker::startServer(AppEnums::ServerType type, quint16 port) {
    const auto key = qMakePair(type, port);

    if (m_servers.contains(key)) {
        m_servers[key]->startServer(port);
        handleLogMessage(QString("Сервер перезапущен (%1:%2).")
                             .arg(AppEnums::typeToString(type))
                             .arg(port));
        return;
    }

    IServer* server = ServerFactory::createServer(type);
    if (!server) {
        handleLogMessage("Ошибка: не удалось создать сервер.");
        emit serverStatusUpdate(type, port, AppEnums::ServerStatus::ERROR, 0);
        return;
    }

    m_dataProcessing->addServer(server);
    m_servers[key] = server;

    server->startServer(port);
    handleLogMessage(QString("Сервер запущен (%1:%2).")
                         .arg(AppEnums::typeToString(type))
                         .arg(port));
    emit serverStatusUpdate(type, port, AppEnums::ServerStatus::RUNNING, 0);
    if (!m_batchTimer->isActive()) {
        m_batchTimer->start(BATCH_TIMEOUT_MS);
    }
}

void ServerWorker::stopServer(AppEnums::ServerType type, quint16 port) {
    const auto key = qMakePair(type, port);

    if (m_servers.contains(key)) {
        IServer* server = m_servers[key];
        server->stopServer();

        handleLogMessage(QString("Сервер на порту %1 остановлен.").arg(port));
        emit serverStatusUpdate(type, port, AppEnums::ServerStatus::STOPPED, 0);
    }
}

void ServerWorker::deleteServer(AppEnums::ServerType type, quint16 port)
{
    const auto key = qMakePair(type, port);

    if (m_servers.contains(key)) {
        IServer* server = m_servers.take(key);
        removeDisconnectedClients();
        server->deleteLater();
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
