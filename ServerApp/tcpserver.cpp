#include "tcpserver.h"
#include "tcpclient.h" // Убедитесь, что этот include есть
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

TcpServer::TcpServer(QObject *parent)
    : IServer(parent), m_tcpServer(nullptr) {
}

TcpServer::~TcpServer() {
}

int TcpServer::clientCount() const {
    return m_clients.size();
}

void TcpServer::startServer(quint16 port) {
    if (m_tcpServer && m_tcpServer->isListening()) {
        emit logMessage("Сервер уже запущен.");
        return;
    }

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::handleNewConnection);

    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        emit logMessage(QString("Ошибка запуска сервера: %1").arg(m_tcpServer->errorString()));
        m_tcpServer->deleteLater();
        m_tcpServer = nullptr;
    } else {
        emit logMessage(QString("Сервер запущен на порту %1").arg(port));
    }
}

void TcpServer::stopServer() {
    if (!m_tcpServer || !m_tcpServer->isListening()) {
        emit logMessage("Сервер уже остановлен.");
        return;
    }

    m_tcpServer->close();
    m_tcpServer->deleteLater();
    m_tcpServer = nullptr;

    m_clients.clear();

    emit logMessage("Сервер остановлен.");
}

void TcpServer::handleNewConnection() {
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket* clientSocket = m_tcpServer->nextPendingConnection();
        if (!clientSocket) {
            continue;
        }

        TcpClient* client = new TcpClient(clientSocket, this);
        quintptr descriptor = client->descriptor();
        client->setId(QString::number(descriptor));

        m_clients.insert(descriptor, client);

        connect(client, &TcpClient::dataReceived, this, &TcpServer::handleDataReceived);
        connect(client, &TcpClient::disconnected, this, &TcpServer::handleClientDisconnected);

        emit clientConnected(client);
        emit logMessage(QString("Новый клиент подключен: %1").arg(client->id()));
    }
}

void TcpServer::handleDataReceived(const QByteArray &data) {
    TcpClient* client = qobject_cast<TcpClient*>(sender());
    if (!client) {
        emit logMessage("Неизвестный отправитель сигнала.");
        return;
    }

    emit dataReceived(client, data);
    emit logMessage(QString("Получены данные от клиента %1").arg(client->id()));
}

void TcpServer::handleClientDisconnected() {
    TcpClient* client = qobject_cast<TcpClient*>(sender());
    if (client) {
        emit clientDisconnected(client);
        emit logMessage(QString("Клиент отключен: %1").arg(client->id()));
    } else {
        emit logMessage("Невозможно определить отключившегося клиента.");
    }
}

void TcpServer::removeClient(IClient* client) {
    if (!client) return;

    quintptr descriptor = 0;
    QString disconnectedId = client->id(); // Получаем ID отключенного клиента

    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.value() && it.value()->id() == disconnectedId) {
            descriptor = it.key();
            break;
        }
    }

    if (m_clients.contains(descriptor)) {
        m_clients.remove(descriptor);
        client->deleteLater();
        emit logMessage(QString("Объект клиента %1 полностью удален.").arg(descriptor));
    }
}

void TcpServer::sendToClient(IClient* client, const QByteArray &data) {
    if (!client || !client->isConnected()) {
        emit logMessage("Клиент не найден или не подключен.");
        return;
    }

    client->sendData(data);
}
