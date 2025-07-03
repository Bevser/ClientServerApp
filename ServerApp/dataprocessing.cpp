#include "dataprocessing.h"

DataProcessing::DataProcessing(QObject *parent)
    : QObject(parent) {

}

DataProcessing::~DataProcessing() {}

void DataProcessing::addServer(IServer* server) {
    if (!server) return;

    connect(server, &IServer::clientConnected,      this, &DataProcessing::handleClientConnected);
    connect(server, &IServer::clientDisconnected,   this, &DataProcessing::handleClientDisconnected);
    connect(server, &IServer::dataReceived,         this, &DataProcessing::handleDataReceived);
}

void DataProcessing::handleClientConnected(IClient* client) {
    if (!client) return;

    quintptr descriptor = client->descriptor();
    client->setId(QString::number(descriptor));

    ClientState state;
    state.client            = client;
    state.status            = AppEnums::AUTHORIZING;
    state.allowSending      = false;

    m_clients[descriptor]   = state;

    m_clientBatch.append(getClientDataMap(state));
    emit logMessage(QString("Клиент %1 (%2:%3) подключен.").
                    arg(client->id()).arg(client->address()).arg(client->port()));
}

QList<QVariantMap> DataProcessing::takeClientUpdatesBatch() {
    QList<QVariantMap> batch;
    if (!m_clientBatch.isEmpty()) {
        batch.swap(m_clientBatch);
    }
    return batch;
}

QList<QVariantMap> DataProcessing::takeDataBatch() {
    QList<QVariantMap> batch;
    if (!m_dataBatch.isEmpty()) {
        batch.swap(m_dataBatch);
    }
    return batch;
}

void DataProcessing::handleClientDisconnected(IClient* client) {
    if (!client) return;

    if (m_clients.contains(client->descriptor())) {
        ClientState& state = m_clients[client->descriptor()];
        state.status        = AppEnums::DISCONNECTED;
        state.allowSending  = false;

        m_clientBatch.append(getClientDataMap(state));
        emit logMessage(QString("Клиент %1 (%2:%3) отключен.").
                        arg(client->id()).arg(client->address()).arg(client->port()));
    } else {
        emit logMessage(QString("Получен сигнал отключения для незарегистрированного клиента."));
    }
}

void DataProcessing::removeDisconnectedClients() {
    int count = 0;

    auto it = m_clients.begin();

    while (it != m_clients.end()) {
        if (it.value().status == AppEnums::DISCONNECTED) {
            IClient* client = it.value().client;
            m_usedClientIds.remove(client->id());
            it.value().status = AppEnums::DELETED;
            m_clientBatch.append(getClientDataMap(it.value()));

            if (IServer* server = qobject_cast<IServer*>(client->parent())) {
                server->removeClient(client);
            }

            it = m_clients.erase(it);
            count++;
        } else {
            ++it;
        }
    }

    emit logMessage(QString("Удалено %1 отключенных клиентов.").arg(count));
}

void DataProcessing::clearClients()
{
    m_clients.clear();
}

QVariantMap DataProcessing::getClientDataMap(const ClientState& state) {
    QVariantMap clientData;
    clientData[Keys::ID]            = state.client->id();
    clientData[Keys::DESCRIPTOR]    = state.client->descriptor();
    clientData[Keys::ADDRESS]       = QString("%1:%2").arg(state.client->address()).arg(state.client->port());
    clientData[Keys::PORT]          = state.client->port();
    clientData[Keys::STATUS]        = state.status;
    clientData[Keys::ALLOW_SENDING] = state.allowSending;
    clientData[Keys::CONFIGURATION] = state.configuration;
    return clientData;
}

void DataProcessing::handleDataReceived(IClient* client, const QByteArray &data) {
    if (!client) return;

    // Можно сделать проверку на json и в случае чего добавить другие парсеры
    parseJsonData(client, data);
}

void DataProcessing::parseJsonData(IClient* client, const QByteArray &data) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit logMessage(QString("Ошибка парсинга JSON от клиента %1: %2").arg(client->id()).arg(parseError.errorString()));
        return;
    }

    if (!doc.isObject()) {
        emit logMessage(QString("Получены некорректные данные от клиента %1.").arg(client->id()));
        return;
    }

    QJsonObject json    = doc.object();
    QString messageType = json[Protocol::Keys::TYPE].toString();
    QJsonObject payload = json[Protocol::Keys::PAYLOAD].toObject();

    if (messageType == Protocol::MessageType::REGISTRATION) {
        QString requestedId = json[Keys::ID].toString();
        registerClient(client, requestedId, payload);
    } else if (m_clients.contains(client->descriptor())) {
        QString clientId = m_clients[client->descriptor()].client->id();
        ClientState& state = m_clients[client->descriptor()];
        state.client = client;

        if (messageType == Protocol::MessageType::NETWORK_METRICS) {
            // Метрики
        } else if (messageType == Protocol::MessageType::DEVICE_STATUS) {
            // Статус
        } else if (messageType == Protocol::MessageType::LOG) {
            // Логи
        } else if (messageType == Protocol::MessageType::CONFIGURATION) {
            // Конфигурация
            state.configuration = payload.toVariantMap();
            emit logMessage(QString("Конфигурация клиента %1 обновлена клиентом.").arg(clientId));
        }

        QVariantMap messageData;
        messageData[Keys::TIME_STAMP]   = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        messageData[Keys::ID]           = clientId;
        messageData[Keys::TYPE]         = messageType;
        messageData[Keys::PAYLOAD]      = payload.toVariantMap();
        m_dataBatch.append(messageData);
    } else {
        emit logMessage(QString("Получены данные от незарегистрированного клиента %1 типа %2").arg(client->descriptor()).arg(messageType));
    }
}

void DataProcessing::registerClient(IClient* client, const QString &id, const QJsonObject &payload) {
    if (!client) {
        emit logMessage("Попытка зарегистрировать null-клиента.");
        return;
    }

    quintptr descriptor = client->descriptor();

    if (!m_clients.contains(descriptor)) {
        emit logMessage(QString("Попытка зарегистрировать клиент повторно (дескриптор: %1) с запрошенным ID: %2")
                            .arg(descriptor).arg(id));
        return;
    }

    ClientState& state = m_clients[descriptor];
    QString oldId = client->id();
    QString newId = id;

    if (state.status == AppEnums::ClientStatus::CONNECTED && state.client->id() == id) {
        emit logMessage(QString("Клиент с ID '%1' уже зарегистрирован").arg(id));
    } else {
        // Проверка id на дублирование
        int suffix = 0;

        while (m_usedClientIds.contains(newId)) {
            suffix++;
            newId = QString("%1_%2").arg(id).arg(suffix);
        }

        state.status        = AppEnums::ClientStatus::CONNECTED;
        state.allowSending  = true;
        state.configuration = payload.toVariantMap();
        client->setId(newId);
        m_usedClientIds.insert(newId);

        m_clientBatch.append(getClientDataMap(m_clients[descriptor]));

        emit logMessage(QString("Клиент %1 (%2:%3) успешно зарегистрирован с ID: %4")
                            .arg(oldId)
                            .arg(state.client->address()).arg(state.client->port())
                            .arg(newId));
    }

    QJsonObject jsonData;
    jsonData[Protocol::Keys::ID]    = newId;
    jsonData[Protocol::Keys::TYPE]  = Protocol::MessageType::CONFIRMATION;

    sendDataToClient(client, QJsonDocument(jsonData).toJson(QJsonDocument::Compact));
}

void DataProcessing::routeDataToClient(const QVariantMap &data) {

    quintptr dc = data[Keys::DESCRIPTOR].toULongLong();
    if (m_clients.contains(dc)) {
        ClientState& state = m_clients[dc];

        if (data[Keys::TYPE] == Keys::CONFIGURATION)
        {
            m_clients[dc].allowSending  = data[Keys::ALLOW_SENDING].toBool();
            m_clients[dc].configuration = data[Keys::PAYLOAD].toMap();
            m_clientBatch.append(getClientDataMap(state));
        }

        QJsonDocument jsonDoc = QJsonDocument::fromVariant(data);
        QByteArray byteArrayData = jsonDoc.toJson(QJsonDocument::Compact);

        sendDataToClient(state.client, byteArrayData);

    } else {
        emit logMessage(QString("Не удалось сохранить конфигурацию: клиент %1 не найден.").arg(data[Keys::ID].toString()));
    }
}

void DataProcessing::sendDataToClient(IClient* client, const QByteArray& data) {
    if (!client) return;

    if (IServer* server = qobject_cast<IServer*>(client->parent())) {
        server->sendToClient(client, data);
    } else {
        emit logMessage("Не удалось найти сервер для отправки данных.");
    }
}

void DataProcessing::sendDataToAll(const QString &data) {
    int count = 0;

    QJsonObject jsonData;
    jsonData[Protocol::Keys::TYPE]      = Protocol::MessageType::COMMAND;
    jsonData[Protocol::Keys::COMMAND]   = data;

    for (const auto& state : qAsConst(m_clients)) {
        if (state.allowSending && state.client && state.client->isConnected()) {
            sendDataToClient(state.client,
                             QJsonDocument(jsonData).toJson(QJsonDocument::Compact));
            count++;
        }
    }
    emit logMessage(QString("Данные отправлены %1 клиентам.").arg(count));
}
