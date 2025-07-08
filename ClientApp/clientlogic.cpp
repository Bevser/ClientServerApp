#include "ClientLogic.h"

ClientLogic::ClientLogic(const QString &host, quint16 port, QObject *parent)
    : QObject(parent), m_host(host), m_port(port), m_client(nullptr),
    m_isStarted(false) {

    // Инициализация таймеров
    m_reconnectTimer    = new QTimer(this);
    m_dataSendTimer     = new QTimer(this);

    // Подключение таймеров к слотам
    connect(m_reconnectTimer, &QTimer::timeout, this,
            &ClientLogic::connectToServer);
    connect(m_dataSendTimer, &QTimer::timeout, this,
            &ClientLogic::sendPeriodicData);
}

ClientLogic::~ClientLogic() {}

void ClientLogic::start() {
    qInfo() << Protocol::LogMessages::CLIENT_STARTING;
    connectToServer();
}

void ClientLogic::connectToServer() {
    // Проверяем, нужно ли создавать нового клиента
    if (!m_client) {
        // Создаем сокет
        QTcpSocket *socket = new QTcpSocket();
        m_client = new TcpClient(socket, this);

        // Подключаем сигналы от клиента
        setupClientConnections();
    }

    qInfo() << QString(Protocol::LogMessages::CONNECTION_ATTEMPT)
                   .arg(m_host)
                   .arg(m_port);
    // Запускаем подключение
    m_client->connectToHost(m_host, m_port);

}

void ClientLogic::setupClientConnections() {
    connect(m_client, &IClient::connected, this, &ClientLogic::handleClientConnected);
    connect(m_client, &IClient::disconnected, this, &ClientLogic::handleClientDisconnected);
    connect(m_client, &IClient::dataReceived, this, &ClientLogic::handleDataReceived);
    connect(m_client, &IClient::errorOccurred, this, &ClientLogic::handleClientError);
}

void ClientLogic::handleClientConnected() {
    qInfo() << Protocol::LogMessages::CONNECTED_SUCCESS;
    m_reconnectTimer->stop();  // Останавливаем таймер переподключения
    sendRegistrationRequest(); // Отправляем запрос на регистрацию
}

void ClientLogic::handleClientDisconnected() {
    qWarning() << Protocol::LogMessages::DISCONNECTED;
    m_dataSendTimer->stop();
    m_isStarted = false;

    // Запускаем таймер для переподключения через 5 секунд
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start(Protocol::Constants::RECONNECT_TIMEOUT);
    }
}

void ClientLogic::handleClientError(const QString &error) {
    qWarning() << Protocol::LogMessages::SOCKET_ERROR << error;
    handleClientDisconnected(); // Запускаем ту же логику, что и при дисконнекте
}

void ClientLogic::handleDataReceived(const QByteArray &data) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << Protocol::LogMessages::INVALID_JSON << error.errorString();
        return;
    }

    if (doc.isObject()) {
        QJsonObject json = doc.object();
        // Обработка подтверждения регистрации
        if (json[Protocol::Keys::TYPE].toString() ==
            Protocol::MessageType::CONFIRMATION) {
            m_client->setId(json[Protocol::Keys::ID].toString());
            qInfo() << Protocol::LogMessages::CONNECTION_CONFIRMED << m_client->id();
            qInfo() << Protocol::LogMessages::WAITING_START;
        }
        // Обработка команд от сервера
        else if (json.contains(Protocol::Keys::COMMAND)) {
            QString command = json[Protocol::Keys::COMMAND].toString();
            if (command == Protocol::Commands::START && !m_isStarted) {
                qInfo() << Protocol::LogMessages::START_RECEIVED;
                m_isStarted = true;
                sendPeriodicData(); // Начинаем отправку немедленно
            } else if (command == Protocol::Commands::STOP) {
                qInfo() << Protocol::LogMessages::STOP_RECEIVED;
                m_isStarted = false;
                m_dataSendTimer->stop();
            }
        }
        // Обработка конфигурации
        else if (json[Protocol::Keys::TYPE].toString() ==
                 Protocol::MessageType::CONFIGURATION) {
            if (json.contains(Protocol::Keys::PAYLOAD)) {
                QJsonObject payload = json[Protocol::Keys::PAYLOAD].toObject();
                m_config.loadFromJson(payload);
                qInfo() << Protocol::LogMessages::CONFIG_RECEIVED;
                qInfo() << Protocol::Keys::MAX_CPU_TEMP + ":" << m_config.maxCpuTemp;
                qInfo() << Protocol::Keys::MAX_CPU_USAGE + ":" << m_config.maxCpuUsage;
                qInfo() << Protocol::Keys::MAX_MEMORY_USAGE + ":" << m_config.maxMemoryUsage;
                qInfo() << Protocol::Keys::MAX_BAND_WIDTH + ":" << m_config.maxBandWidth;
                qInfo() << Protocol::Keys::MAX_LATENCY + ":" << m_config.maxLatency;
                qInfo() << Protocol::Keys::MAX_PACKET_LOSS + ":" << m_config.maxPacketLoss;
            }
        }
    }
}

void ClientLogic::sendRegistrationRequest() {
    QJsonObject data;
    data[Protocol::Keys::ID] = Protocol::Constants::CLIENT_ID; // Запрашиваемый ID
    data[Protocol::Keys::TYPE] = Protocol::MessageType::REGISTRATION;

    // Добавляем текущую конфигурацию
    data[Protocol::Keys::PAYLOAD] = m_config.toJson();
    sendJson(data);
}

void ClientLogic::sendPeriodicData() {
    if (!m_isStarted || !m_client || !m_client->isConnected()) {
        m_dataSendTimer->stop();
        return;
    }

    // Циклически меняем тип отправляемых данных
    static int dataType = 0;
    dataType == 2 ? dataType = 0 : dataType++;

    QJsonObject data;
    switch (dataType) {
    case 0:
        data = generateNetworkMetrics();
        break;
    case 1:
        data = generateDeviceStatus();
        break;
    case 2:
        data = generateLog();
        break;
    }

    data[Protocol::Keys::ID] =
        m_client->id(); // Добавляем наш ID в каждое сообщение

    // Проверяем пороговые значения и изменяем severity если нужно
    checkThresholds(data);

    sendJson(data);

    // Устанавливаем случайную задержку для следующей отправки
    int delay = QRandomGenerator::global()->bounded(Protocol::Constants::MIN_DELAY, Protocol::Constants::MAX_DELAY);
    m_dataSendTimer->start(delay);
}

void ClientLogic::sendJson(const QJsonObject &json) {
    if (m_client && m_client->isConnected()) {
        QJsonDocument doc(json);
        m_client->sendData(doc.toJson(QJsonDocument::Compact));
    }
}

bool ClientLogic::checkThresholds(QJsonObject &data) {
    if (!data.contains(Protocol::Keys::PAYLOAD)) {
        return false;
    }

    QJsonObject payload = data[Protocol::Keys::PAYLOAD].toObject();
    QString messageType = data[Protocol::Keys::TYPE].toString();

    QStringList criticalMessages;

    if (messageType == Protocol::MessageType::NETWORK_METRICS) {
        checkNetworkMetrics(payload, criticalMessages);
    } else if (messageType == Protocol::MessageType::DEVICE_STATUS) {
        checkDeviceStatus(payload, criticalMessages);
    } else if (messageType == Protocol::MessageType::LOG) {
        // Для логов не проверяем пороговые значения
        return false;
    }

    // Если есть критические превышения, модифицируем сообщение
    if (!criticalMessages.isEmpty()) {
        // Меняем тип сообщения на LOG с критическим уровнем
        data[Protocol::Keys::TYPE] = Protocol::MessageType::LOG;

        // Добавляем информацию о критических превышениях в payload
        payload[Protocol::Keys::SEVERITY] = Protocol::Severity::CRITICAL;
        payload[Protocol::Keys::MESSAGE] = Protocol::Messages::THRESHOLD_EXCEEDED + criticalMessages.join("; ");

        data[Protocol::Keys::PAYLOAD] = payload;

        return true;
    }

    return false;
}

void ClientLogic::checkNetworkMetrics(const QJsonObject &payload, QStringList &criticalMessages) {
    // Проверяем пропускную способность
    if (payload.contains(Protocol::Keys::BAND_WIDTH)) {
        double bandWidth = payload[Protocol::Keys::BAND_WIDTH].toString().toDouble();
        if (bandWidth > m_config.maxBandWidth) {
            criticalMessages.append(QString("%1: %2 > %3")
                                        .arg(Protocol::Keys::BAND_WIDTH)
                                        .arg(bandWidth, 0, 'f', 2)
                                        .arg(m_config.maxBandWidth, 0, 'f', 2));
        }
    }

    // Проверяем задержку
    if (payload.contains(Protocol::Keys::LATENCY)) {
        double latency = payload[Protocol::Keys::LATENCY].toString().toDouble();
        if (latency > m_config.maxLatency) {
            criticalMessages.append(QString("%1: %2 > %3")
                                        .arg(Protocol::Keys::LATENCY)
                                        .arg(latency, 0, 'f', 2)
                                        .arg(m_config.maxLatency, 0, 'f', 2));
        }
    }

    // Проверяем потерю пакетов
    if (payload.contains(Protocol::Keys::PACKET_LOSS)) {
        double packetLoss = payload[Protocol::Keys::PACKET_LOSS].toString().toDouble();
        if (packetLoss > m_config.maxPacketLoss) {
            criticalMessages.append(QString("%1: %2 > %3")
                                        .arg(Protocol::Keys::PACKET_LOSS)
                                        .arg(packetLoss, 0, 'f', 2)
                                        .arg(m_config.maxPacketLoss, 0, 'f', 2));
        }
    }
}

void ClientLogic::checkDeviceStatus(const QJsonObject &payload, QStringList &criticalMessages) {
    // Проверяем загрузку процессора
    if (payload.contains(Protocol::Keys::CPU_USAGE)) {
        double cpuUsage = payload[Protocol::Keys::CPU_USAGE].toDouble();
        if (cpuUsage > m_config.maxCpuUsage) {
            criticalMessages.append(QString("%1: %2 > %3")
                                        .arg(Protocol::Keys::CPU_USAGE)
                                        .arg(cpuUsage, 0, 'f', 2)
                                        .arg(m_config.maxCpuUsage, 0, 'f', 2));
        }
    }

    // Проверяем загрузку памяти
    if (payload.contains(Protocol::Keys::MEMORY_USAGE)) {
        double memoryUsage = payload[Protocol::Keys::MEMORY_USAGE].toDouble();
        if (memoryUsage > m_config.maxMemoryUsage) {
            criticalMessages.append(QString("%1: %2 > %3")
                                        .arg(Protocol::Keys::MEMORY_USAGE)
                                        .arg(memoryUsage, 0, 'f', 2)
                                        .arg(m_config.maxMemoryUsage, 0, 'f', 2));
        }
    }

    // Проверяем температуру процессора
    if (payload.contains(Protocol::Keys::CPU_TEMP)) {
        double cpuTemp = payload[Protocol::Keys::CPU_TEMP].toDouble();
        if (cpuTemp > m_config.maxCpuTemp) {
            criticalMessages.append(QString("%1: %2 > %3")
                                        .arg(Protocol::Keys::CPU_TEMP)
                                        .arg(cpuTemp, 0, 'f', 2)
                                        .arg(m_config.maxCpuTemp, 0, 'f', 2));
        }
    }
}

// --- Методы генерации данных ---

QJsonObject ClientLogic::generateNetworkMetrics() {
    QJsonObject metrics;
    metrics[Protocol::Keys::TYPE] = Protocol::MessageType::NETWORK_METRICS;
    QJsonObject payload;
    payload[Protocol::Keys::BAND_WIDTH] = QString::number(QRandomGenerator::global()->generateDouble() * 1200, 'f', 2);
    payload[Protocol::Keys::LATENCY] = QString::number(QRandomGenerator::global()->generateDouble() * 150, 'f', 2);
    payload[Protocol::Keys::PACKET_LOSS] = QString::number(QRandomGenerator::global()->bounded(0, 8) / 100.0, 'f', 2);
    metrics[Protocol::Keys::PAYLOAD] = payload;
    return metrics;
}

QJsonObject ClientLogic::generateDeviceStatus() {
    QJsonObject status;
    status[Protocol::Keys::TYPE] = Protocol::MessageType::DEVICE_STATUS;
    QJsonObject payload;
    payload[Protocol::Keys::UP_TIME]        = QRandomGenerator::global()->bounded(1, 100000);
    payload[Protocol::Keys::CPU_USAGE]      = QRandomGenerator::global()->bounded(5, 100);
    payload[Protocol::Keys::MEMORY_USAGE]   = QRandomGenerator::global()->bounded(10, 100);
    payload[Protocol::Keys::CPU_TEMP]       = QRandomGenerator::global()->bounded(10, 95);
    status[Protocol::Keys::PAYLOAD]         = payload;
    return status;
}

QJsonObject ClientLogic::generateLog() {
    QJsonObject log;
    log[Protocol::Keys::TYPE] = Protocol::MessageType::LOG;

    QStringList messages = {Protocol::Messages::HIGH_CPU_TEMP,
                            Protocol::Messages::CONFIG_UPDATED,
                            Protocol::Messages::DB_CONNECT_FAILED};
    QStringList severities = {Protocol::Severity::INFO,
                              Protocol::Severity::WARN,
                              Protocol::Severity::ERROR,
                              Protocol::Severity::CRITICAL};
    QJsonObject payload;

    // Генерация содержимого
    QString junk;
    for (int i = 0; i < Protocol::Constants::JUNK_LENGTH; ++i) {
        int index = QRandomGenerator::global()->bounded(Protocol::Constants::JUNK_CHARS.length());
        junk.append(Protocol::Constants::JUNK_CHARS.at(index));
    }

    payload[Protocol::Keys::JUNK]       = junk;
    payload[Protocol::Keys::SEVERITY]   = severities.at(QRandomGenerator::global()->bounded(severities.size()));
    payload[Protocol::Keys::MESSAGE]    = messages.at(QRandomGenerator::global()->bounded(messages.size()));
    log[Protocol::Keys::PAYLOAD]        = payload;
    return log;
}

