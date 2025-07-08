#include "ClientLogic.h"

ClientLogic::ClientLogic(const QString &host, quint16 port, QObject *parent)
    : QObject(parent), m_host(host), m_port(port), m_client(nullptr),
    m_isStarted(false) {
    // Инициализация таймеров
    m_reconnectTimer = new QTimer(this);
    m_dataSendTimer = new QTimer(this);

    // Подключение таймеров к слотам
    connect(m_reconnectTimer, &QTimer::timeout, this,
            &ClientLogic::connectToServer);
    connect(m_dataSendTimer, &QTimer::timeout, this,
            &ClientLogic::sendPeriodicData);
}

ClientLogic::~ClientLogic() {}

void ClientLogic::start() {
    qInfo() << "[ИНФО] Клиент запускается...";
    connectToServer();
}

void ClientLogic::connectToServer() {
    // Проверяем, нужно ли создавать нового клиента
    if (!m_client || !m_client->isConnected()) {
        qInfo() << QString("[ИНФО] Попытка подключения к %1:%2...")
                       .arg(m_host)
                       .arg(m_port);

        // Создаем сокет и оборачиваем его в TcpClient
        QTcpSocket *socket = new QTcpSocket();
        m_client = new TcpClient(socket, this);

        // Подключаем сигналы от клиента к нашим слотам
        setupClientConnections();

        // Запускаем подключение
        m_client->connectToHost(m_host, m_port);
    }
}

void ClientLogic::setupClientConnections() {
    connect(m_client, &IClient::connected, this, &ClientLogic::handleClientConnected);
    connect(m_client, &IClient::disconnected, this,
            &ClientLogic::handleClientDisconnected);
    connect(m_client, &IClient::dataReceived, this, &ClientLogic::handleDataReceived);
    connect(m_client, &IClient::errorOccurred, this, &ClientLogic::handleClientError);
}

void ClientLogic::handleClientConnected() {
    qInfo() << "[OK] Успешно подключено к серверу.";
    m_reconnectTimer->stop();  // Останавливаем таймер переподключения
    sendRegistrationRequest(); // Отправляем запрос на регистрацию
}

void ClientLogic::handleClientDisconnected() {
    qWarning() << "[ОШИБКА] Соединение с сервером потеряно.";
    m_dataSendTimer->stop();
    m_isStarted = false;

    // Запускаем таймер для переподключения через 5 секунд
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start(5000);
    }
}

void ClientLogic::handleClientError(const QString &error) {
    qWarning() << "[ОШИБКА] Ошибка сокета:" << error;
    handleClientDisconnected(); // Запускаем ту же логику, что и при дисконнекте
}

void ClientLogic::handleDataReceived(const QByteArray &data) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "[ОШИБКА] Неверный JSON от сервера:" << error.errorString();
        return;
    }

    if (doc.isObject()) {
        QJsonObject json = doc.object();
        // Обработка подтверждения регистрации
        if (json[Protocol::Keys::TYPE].toString() ==
            Protocol::MessageType::CONFIRMATION) {
            m_client->setId(json[Protocol::Keys::ID].toString());
            qInfo() << "[OK] Подключение подтверждено. Мой ID:" << m_client->id();
            qInfo() << "[ИНФО] Ожидание команды 'start' от сервера...";
        }
        // Обработка команд от сервера
        else if (json.contains(Protocol::Keys::COMMAND)) {
            QString command = json[Protocol::Keys::COMMAND].toString();
            if (command == Protocol::Commands::START && !m_isStarted) {
                qInfo() << "[СТАРТ] Получена команда 'start'. Начинаю отправку данных.";
                m_isStarted = true;
                sendPeriodicData(); // Начинаем отправку немедленно
            } else if (command == Protocol::Commands::STOP) {
                qInfo() << "[СТОП] Получена команда 'stop'. Прекращаю отправку данных.";
                m_isStarted = false;
                m_dataSendTimer->stop();
            }
        }
    }
}

void ClientLogic::sendRegistrationRequest() {
    QJsonObject data;
    data[Protocol::Keys::ID] = "Client"; // Запрашиваемый ID
    data[Protocol::Keys::TYPE] = Protocol::MessageType::REGISTRATION;

    // Добавляем полезную нагрузку (конфигурацию)
    QJsonObject payload;
    payload["bandwidth"] = QString::number(
        QRandomGenerator::global()->generateDouble() * 1000, 'f', 2);
    payload["latency"] = QString::number(
        QRandomGenerator::global()->generateDouble() * 100, 'f', 2);
    payload["packet_loss"] = QString::number(
        QRandomGenerator::global()->bounded(0, 5) / 100.0, 'f', 2);
    data[Protocol::Keys::PAYLOAD] = payload;

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
    sendJson(data);

    // Устанавливаем случайную задержку для следующей отправки
    int delay = QRandomGenerator::global()->bounded(100, 1000);
    m_dataSendTimer->start(delay);
}

void ClientLogic::sendJson(const QJsonObject &json) {
    if (m_client && m_client->isConnected()) {
        QJsonDocument doc(json);
        m_client->sendData(doc.toJson(QJsonDocument::Compact));
    }
}

// --- Методы генерации данных ---

QJsonObject ClientLogic::generateNetworkMetrics() {
    QJsonObject metrics;
    metrics[Protocol::Keys::TYPE] = Protocol::MessageType::NETWORK_METRICS;
    QJsonObject payload;
    payload["bandwidth"] = QString::number(
        QRandomGenerator::global()->generateDouble() * 1000, 'f', 2);
    payload["latency"] = QString::number(
        QRandomGenerator::global()->generateDouble() * 100, 'f', 2);
    payload["packet_loss"] = QString::number(
        QRandomGenerator::global()->bounded(0, 5) / 100.0, 'f', 2);
    metrics[Protocol::Keys::PAYLOAD] = payload;
    return metrics;
}

QJsonObject ClientLogic::generateDeviceStatus() {
    QJsonObject status;
    status[Protocol::Keys::TYPE] = Protocol::MessageType::DEVICE_STATUS;
    QJsonObject payload;
    payload["uptime"] = QRandomGenerator::global()->bounded(1, 100000);
    payload["cpu_usage"] = QRandomGenerator::global()->bounded(5, 95);
    payload["memory_usage"] = QRandomGenerator::global()->bounded(10, 98);
    payload["cpu_temp"] = QRandomGenerator::global()->bounded(10, 90);
    status[Protocol::Keys::PAYLOAD] = payload;
    return status;
}

QJsonObject ClientLogic::generateLog() {
    QJsonObject log;
    log[Protocol::Keys::TYPE] = Protocol::MessageType::LOG;
    QStringList messages = {"High CPU temperature detected",
                            "Configuration updated successfully",
                            "Failed to connect to database."};
    QStringList severities = {"INFO", "WARN", "ERROR", "CRITICAL"};
    QJsonObject payload;
    payload["severity"] =
        severities.at(QRandomGenerator::global()->bounded(severities.size()));
    payload["message"] =
        messages.at(QRandomGenerator::global()->bounded(messages.size()));
    log[Protocol::Keys::PAYLOAD] = payload;
    return log;
}
