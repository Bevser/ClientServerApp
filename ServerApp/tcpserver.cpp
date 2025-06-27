#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent)
    : QObject(parent), m_tcpServer(nullptr)
{
}

TcpServer::~TcpServer()
{
    stopServer();
}

void TcpServer::startServer(quint16 port)
{
    if (m_tcpServer && m_tcpServer->isListening()) {
        emit logMessage("Сервер уже запущен.");
        return;
    }

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &::TcpServer::handleNewConnection);

    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        emit logMessage(QString("Ошибка запуска сервера: %1").arg(m_tcpServer->errorString()));
        delete m_tcpServer;
        m_tcpServer = nullptr;
    } else {
        emit logMessage(QString("Сервер запущен на порту %1").arg(port));
    }
}

void TcpServer::stopServer()
{
    if (!m_tcpServer || !m_tcpServer->isListening()) {
        return;
    }

    // Отключаем всех клиентов
    for (const auto& clientEntry : m_clients) {
        if (clientEntry.client) {
            clientEntry.client->disconnect();
            clientEntry.client->deleteLater();
        }
    }

    m_clients.clear();
    m_tcpServer->close();
    m_tcpServer->deleteLater();
    m_tcpServer = nullptr;

    emit logMessage("Сервер остановлен.");
}

void TcpServer::handleNewConnection()
{
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket* clientSocket = m_tcpServer->nextPendingConnection();
        if (!clientSocket) {
            continue;
        }

        TcpClient* client = new TcpClient(clientSocket, m_tcpServer);

        ClientEntry entry;
        entry.client = client;
        entry.allowSending = true;
        m_clients.append(entry);

        connect(client, &TcpClient::readyRead, this, &TcpServer::handleReadyRead);
        connect(client, &TcpClient::disconnected, this, &TcpServer::handleClientDisconnected);

        quintptr descriptor   = client->descriptor();
        QString ip            = client->peerAddress();
        quint16 port          = client->peerPort();

        emit clientConnected(descriptor, ip, port);
        emit logMessage(QString("Новый клиент подключен: %1:%2").arg(ip).arg(port));

        QJsonObject confirmation;
        confirmation["type"]    = "Confirmation";
        confirmation["id"]      = QString::number(m_clients.size());
        client->sendData(confirmation);
    }
}

void TcpServer::handleReadyRead()
{
    TcpClient* client = qobject_cast<TcpClient*>(sender());
    if (!client) {
        emit logMessage("Невозможно определить отправителя сигнала.");
        return;
    }

    QByteArray data = client->readData();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit logMessage(QString("Ошибка парсинга JSON от %1: %2")
                            .arg(client->descriptor()).arg(parseError.errorString()));
        return;
    }

    if (jsonDoc.isObject()) {
        QJsonObject obj = jsonDoc.object();
        if (obj.contains("id")) {
            // Обновляем ID клиента
            client->id(obj["id"].toString());
        }

        emit dataReceived(client->descriptor(), obj, client->id());
        emit logMessage(QString("Получены данные: %1").arg(client->id()));
    }
}

void TcpServer::handleClientDisconnected()
{
    TcpClient* client = qobject_cast<TcpClient*>(sender());
    if (!client) {
        emit logMessage("Невозможно определить отключившегося клиента.");
        return;
    }

    quintptr descriptor   = client->descriptor();
    QString ip            = client->peerAddress();
    quint16 port          = client->peerPort();

    // Ищем и удаляем клиента из списка
    auto it = std::find_if(m_clients.begin(), m_clients.end(),
                           [client](const ClientEntry& entry) {
                               return entry.client == client;
                           });

    if (it != m_clients.end()) {
        emit clientDisconnected(descriptor, ip, port);
        emit logMessage(QString("Клиент отключен: %1:%2 (descriptor: %3)")
                            .arg(ip).arg(port).arg(descriptor));

        m_clients.erase(it);
    }

    // Удаляем сам объект
    client->deleteLater();
}

QString TcpServer::commandToString(CommandType command) {
    switch (command) {
    case CommandType::Start:        return "start";
    case CommandType::Stop:         return "stop";
    case CommandType::Configure:    return "configure";
    default: return "Unknown";
    }
}

void TcpServer::sendCommandToAll(CommandType command)
{
    if (!m_tcpServer || !m_tcpServer->isListening()) {
        emit logMessage("Сервер не запущен.");
        return;
    }

    QString com = commandToString(command);

    for (const auto& entry : m_clients) {
        if (!entry.allowSending || !entry.client || !entry.client->isConnected())
            continue;

        QJsonObject json;
        json["command"] = com;

        static_cast<TcpClient*>(entry.client)->sendData(json);
    }

    emit logMessage(QString("Команда '%1' отправлена всем клиентам.").arg(com));
}

int TcpServer::clientCount() const
{
    return m_clients.size();
}
