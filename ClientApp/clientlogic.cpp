#include "clientlogic.h"


ClientLogic::ClientLogic(const QString& host, quint16 port, QObject *parent)
    : QObject(parent), m_host(host), m_port(port), m_isStarted(false)
{
    m_socket = new QTcpSocket(this);
    m_reconnectTimer = new QTimer(this);
    m_dataSendTimer = new QTimer(this);

    // Подключаем все необходимые сигналы сокета
    connect(m_socket, &QTcpSocket::connected, this, &ClientLogic::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientLogic::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientLogic::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &ClientLogic::onSocketError);

    // Таймеры
    connect(m_reconnectTimer, &QTimer::timeout, this, &ClientLogic::connectToServer);
    connect(m_dataSendTimer, &QTimer::timeout, this, &ClientLogic::sendData);
}

void ClientLogic::start() {
    qInfo() << "[ИНФО] Клиент запускается...";
    connectToServer();
}

void ClientLogic::connectToServer() {
    if (m_socket->state() == QAbstractSocket::UnconnectedState) {
        qInfo() << QString("[ИНФО] Попытка подключения к %1:%2...").arg(m_host).arg(m_port);
        m_socket->connectToHost(m_host, m_port);
    }
}

void ClientLogic::onConnected() {
    qInfo() << "[OK] Успешно подключено к серверу.";
    // Если мы успешно подключились, останавливаем таймер переподключения
    m_reconnectTimer->stop();

    sendData();
    QJsonObject data;
    data[Protocol::Keys::ID]    = "Client";
    data[Protocol::Keys::TYPE]  = Protocol::MessageType::REGISTRATION;

    QJsonObject payload; // Создаем объект для полезной нагрузки
    payload["bandwidth"]    = QString::number(QRandomGenerator::global()->generateDouble() * 1000, 'f', 2);
    payload["latency"]      = QString::number(QRandomGenerator::global()->generateDouble() * 100, 'f', 2);
    payload["packet_loss"]  = QString::number(QRandomGenerator::global()->bounded(0, 5) / 100.0, 'f', 2);
    data[Protocol::Keys::PAYLOAD] = payload;
    sendJson(data);
}

void ClientLogic::onDisconnected() {
    qWarning() << "[ОШИБКА] Соединение с сервером потеряно.";
    m_dataSendTimer->stop();
    m_isStarted = false;
    // Запускаем таймер для переподключения
    startReconnectTimer();
}

void ClientLogic::onSocketError(QAbstractSocket::SocketError socketError) {
    // Выводим ошибку в консоль
    qWarning() << "[ОШИБКА] Ошибка сокета:" << m_socket->errorString();
    startReconnectTimer();
}

void ClientLogic::startReconnectTimer() {
    // Запускаем таймер
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start(5000);
    }
}

void ClientLogic::onReadyRead() {
    QByteArray data = m_socket->readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "[ОШИБКА] Неверный JSON от сервера:" << error.errorString();
        return;
    }

    if (doc.isObject()) {
        QJsonObject json = doc.object();
        if (json.contains(Protocol::Keys::TYPE) &&
            json[Protocol::Keys::TYPE].toString() == Protocol::MessageType::CONFIRMATION) {
            m_clientId = json[Protocol::Keys::ID].toString();
            qInfo() << "[OK] Подключение подтверждено. Мой ID:" << QString(m_clientId);
            qInfo() << "[ИНФО] Ожидание команды 'start' от сервера...";

        } else if (json.contains(Protocol::Keys::COMMAND)) {
            QString command = json[Protocol::Keys::COMMAND].toString();
            if (command == Protocol::Commands::START && !m_isStarted) {
                qInfo() << "[СТАРТ] Получена команда 'start'. Начинаю отправку данных.";
                m_isStarted = true;
                sendData();
            } else if (command == Protocol::Commands::STOP) {
                qInfo() << "[СТОП] Получена команда 'stop'. Прекращаю отправку данных.";
                m_isStarted = false;
                m_dataSendTimer->stop();
            }
        }
    }
}

void ClientLogic::sendData() {
    if (!m_isStarted || m_socket->state() != QAbstractSocket::ConnectedState) {
        m_dataSendTimer->stop();
        return;
    }

    static int dataType = 0;
    dataType == 2 ? dataType = 0 : dataType++;

    QJsonObject data;
    switch (dataType) {
    case 0: data = generateNetworkMetrics(); break;
    case 1: data = generateDeviceStatus(); break;
    case 2: data = generateLog(); break;
    }
    data[Protocol::Keys::ID] = m_clientId;
    sendJson(data);
    int delay = QRandomGenerator::global()->bounded(100, 1000);
    m_dataSendTimer->start(delay);
}

void ClientLogic::sendJson(const QJsonObject& json) {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(json);
        m_socket->write(doc.toJson(QJsonDocument::Compact));
    }
}

QJsonObject ClientLogic::generateNetworkMetrics() {
    QJsonObject metrics;
    metrics[Protocol::Keys::TYPE] = Protocol::MessageType::NETWORK_METRICS;
    QJsonObject payload; // Создаем объект для полезной нагрузки
    payload["bandwidth"]    = QString::number(QRandomGenerator::global()->generateDouble() * 1000, 'f', 2);
    payload["latency"]      = QString::number(QRandomGenerator::global()->generateDouble() * 100, 'f', 2);
    payload["packet_loss"]  = QString::number(QRandomGenerator::global()->bounded(0, 5) / 100.0, 'f', 2);
    metrics[Protocol::Keys::PAYLOAD] = payload;

    return metrics;
}

QJsonObject ClientLogic::generateDeviceStatus() {
    QJsonObject status;
    status[Protocol::Keys::TYPE] = Protocol::MessageType::DEVICE_STATUS;
    QJsonObject payload; // Создаем объект для полезной нагрузки
    payload["uptime"]        = QRandomGenerator::global()->bounded(1, 100000);
    payload["cpu_usage"]     = QRandomGenerator::global()->bounded(5, 95);
    payload["memory_usage"]  = QRandomGenerator::global()->bounded(10, 98);
    payload["cpu_temp"]      = QRandomGenerator::global()->bounded(10, 90);
    status[Protocol::Keys::PAYLOAD] = payload;
    return status;
}

QJsonObject ClientLogic::generateLog() {
    QJsonObject log;
    log[Protocol::Keys::TYPE] = Protocol::MessageType::LOG;
    QStringList messages = {
        "High CPU temperature detected",
        "Configuration updated successfully",
        "Failed to connect to database."
    };
    QStringList severities = {"INFO", "WARN", "ERROR", "CRITICAL"};
    QJsonObject payload; // Создаем объект для полезной нагрузки
    payload["severity"] = severities.at(QRandomGenerator::global()->bounded(severities.size()));
    payload["message"] = messages.at(QRandomGenerator::global()->bounded(messages.size()));
    log[Protocol::Keys::PAYLOAD] = payload;

    return log;
}
