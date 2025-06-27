#include "clientlogic.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>

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
    qInfo() << "[OK] Успешно подключено к серверу. Ожидание подтверждения...";
    // Если мы успешно подключились, останавливаем таймер переподключения
    m_reconnectTimer->stop();
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
        QJsonObject obj = doc.object();
        if (obj.contains("type") && obj["type"].toString() == "Confirmation") {
            m_clientId = "Client-" + obj["id"].toString();
            qInfo() << "[OK] Подключение подтверждено. Мой ID:" << QString(m_clientId);
            qInfo() << "[ИНФО] Ожидание команды 'start' от сервера...";
            QJsonObject idPacket;
            idPacket["type"] = "Identification";
            idPacket["id"] = m_clientId;
            sendJson(idPacket);
        } else if (obj.contains("command")) {
            QString command = obj["command"].toString();
            if (command == "start" && !m_isStarted) {
                qInfo() << "[СТАРТ] Получена команда 'start'. Начинаю отправку данных.";
                m_isStarted = true;
                sendData();
            } else if (command == "stop") {
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
    int dataType = QRandomGenerator::global()->bounded(0, 3);
    QJsonObject data;
    switch (dataType) {
    case 0: data = generateNetworkMetrics(); break;
    case 1: data = generateDeviceStatus(); break;
    case 2: data = generateLog(); break;
    }
    data["id"] = m_clientId;
    sendJson(data);
    int delay = QRandomGenerator::global()->bounded(100, 1001);
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
    metrics["type"] = "NetworkMetrics";
    metrics["bandwidth"] = QString::number(QRandomGenerator::global()->generateDouble() * 1000, 'f', 2);
    metrics["latency"] = QString::number(QRandomGenerator::global()->generateDouble() * 100, 'f', 2);
    metrics["packet_loss"] = QString::number(QRandomGenerator::global()->bounded(0, 5) / 100.0, 'f', 2);
    return metrics;
}

QJsonObject ClientLogic::generateDeviceStatus() {
    QJsonObject status;
    status["type"] = "DeviceStatus";
    status["uptime"] = QRandomGenerator::global()->bounded(1, 100000);
    status["cpu_usage"] = QRandomGenerator::global()->bounded(5, 95);
    status["memory_usage"] = QRandomGenerator::global()->bounded(10, 98);
    return status;
}

QJsonObject ClientLogic::generateLog() {
    QJsonObject log;
    log["type"] = "Log";
    QStringList messages = {
        "Interface eth0 link is down", "User 'admin' logged in from 192.168.1.100",
        "High CPU temperature detected: 85C",
        "Configuration updated successfully. Service 'firewall' was reloaded.",
        "CRITICAL: Failed to connect to database 'prod_db' after 3 retries. Check connection."
    };
    QStringList severities = {"INFO", "WARN", "ERROR", "CRITICAL"};
    log["message"] = messages.at(QRandomGenerator::global()->bounded(messages.size()));
    log["severity"] = severities.at(QRandomGenerator::global()->bounded(severities.size()));
    return log;
}
