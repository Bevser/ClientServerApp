#include "dataprocessing.h"
#include "core/iserver.h"
#include "core/appenums.h"
#include "core/sharedkeys.h"

DataProcessing::DataProcessing(QObject *parent) : QObject(parent) {}

DataProcessing::~DataProcessing() {}

void DataProcessing::addServer(IServer *server) {
    if (!server)
        return;

    connect(server, &IServer::clientConnected, this, &DataProcessing::handleClientConnected);
    connect(server, &IServer::clientDisconnected, this, &DataProcessing::handleClientDisconnected);
    connect(server, &IServer::dataReceived, this, &DataProcessing::handleDataReceived);
}

void DataProcessing::handleClientConnected(IClient *client) {
    if (!client)
        return;

    quintptr descriptor = client->descriptor();
    client->setId(QString::number(descriptor));

    ClientState state;
    state.client = client;
    state.status = AppEnums::AUTHORIZING;
    state.allowSending = false;

    m_clients[descriptor] = state;

    m_clientBatch.append(getClientDataMap(state));
    emit logMessage(QString("Клиент %1 (%2:%3) ожидает авторизации.")
                        .arg(client->id())
                        .arg(client->address())
                        .arg(client->port()));
}

void DataProcessing::handleClientDisconnected(IClient *client) {
    if (!client)
        return;

    if (m_clients.contains(client->descriptor())) {
        ClientState &state = m_clients[client->descriptor()];
        // Если клиент отключается до регистрации, его можно сразу удалить
        if (state.status == AppEnums::AUTHORIZING) {
            auto it = m_clients.find(client->descriptor());
            if (it != m_clients.end()) {
                removeClient(it.value());
                m_clients.erase(it);
            }
        } else {
            state.status = AppEnums::DISCONNECTED;
            state.allowSending = false;
            m_clientBatch.append(getClientDataMap(state));
        }

        emit logMessage(QString("Клиент %1 (%2:%3) отключен.")
                            .arg(client->id())
                            .arg(client->address())
                            .arg(client->port()));
    } else {
        emit logMessage(QString("Получен сигнал отключения для незарегистрированного клиента."));
    }
}

void DataProcessing::removeDisconnectedClients() {
    int count = 0;
    auto it = m_clients.begin();

    while (it != m_clients.end()) {
        if (it.value().status == AppEnums::DISCONNECTED) {
            removeClient(it.value());
            it = m_clients.erase(it);
            count++;
        } else {
            ++it;
        }
    }

    if (count)
        emit logMessage(QString("Удалено %1 неактивных клиентов.").arg(count));
}

void DataProcessing::removeClient(ClientState &state) {
    IClient *client = state.client;
    if (!client) return;

    m_usedClientIds.remove(client->id());
    state.status = AppEnums::DELETED;
    m_clientBatch.append(getClientDataMap(state));

    if (IServer *server = qobject_cast<IServer *>(client->parent())) {
        server->removeClient(client);
    }
}

void DataProcessing::registerClient(IClient *client, const QString &id, const QJsonObject &payload) {
    if (!client) {
        emit logMessage("Попытка зарегистрировать null-клиента.");
        return;
    }

    quintptr descriptor = client->descriptor();

    if (!m_clients.contains(descriptor)) {
        emit logMessage(QString("Попытка зарегистрировать клиент, который не проходил первичное подключение (дескриптор: %1).").arg(descriptor));
        return;
    }

    QString assignedId = id;
    bool allowSending = true;

    // Поиск клиента с таким же ID в состоянии DISCONNECTED
    auto it = std::find_if(m_clients.begin(), m_clients.end(),
                           [&](const auto &state) {
                               return state.client && state.client->id() == id &&
                                      state.status == AppEnums::DISCONNECTED;
                           });

    if (it != m_clients.end()) {
        // Переподключение
        ClientState oldState = it.value();
        // Удаляем старого клиента
        removeClient(oldState);
        m_clients.erase(it);
        allowSending = oldState.allowSending;
    } else {
        // Новый клиент
        int suffix = 0;
        while (m_usedClientIds.contains(assignedId)) {
            assignedId = QString("%1_%2").arg(id).arg(++suffix);
        }
    }

    // Регистрируем клиента
    ClientState &state = m_clients[descriptor];
    client->setId(assignedId);
    state.client        = client;
    state.status        = AppEnums::CONNECTED;
    state.allowSending  = allowSending;
    state.configuration = payload.toVariantMap();
    m_usedClientIds.insert(assignedId);

    m_clientBatch.append(getClientDataMap(state));

    emit logMessage(QString("Клиент %1 (%2:%3) успешно зарегистрирован с ID: %4")
                        .arg(QString::number(descriptor)).arg(client->address()).arg(client->port()).arg(assignedId));


    // Подтверждение регистрации
    QJsonObject jsonData;
    jsonData[Protocol::Keys::ID] = client->id();
    jsonData[Protocol::Keys::TYPE] = Protocol::MessageType::CONFIRMATION;
    sendDataToClient(client, QJsonDocument(jsonData).toJson(QJsonDocument::Compact));
}

void DataProcessing::clearClients() {
    m_clients.clear();
    m_usedClientIds.clear();
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

QVariantMap DataProcessing::getClientDataMap(const ClientState &state) {
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

void DataProcessing::handleDataReceived(IClient *client, const QByteArray &data) {
    if (!client) return;
    parseJsonData(client, data);
}

void DataProcessing::parseJsonData(IClient *client, const QByteArray &data) {
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

    QJsonObject json = doc.object();
    QString messageType = json[Protocol::Keys::TYPE].toString();
    QJsonObject payload = json[Protocol::Keys::PAYLOAD].toObject();

    if (messageType == Protocol::MessageType::REGISTRATION) {
        QString requestedId = json[Keys::ID].toString();
        registerClient(client, requestedId, payload);
    } else if (m_clients.contains(client->descriptor())) {
        ClientState &state = m_clients[client->descriptor()];
        if (messageType == Protocol::MessageType::CONFIGURATION) {
            state.configuration = payload.toVariantMap();
            m_clientBatch.append(getClientDataMap(state));
            emit logMessage(QString("Конфигурация клиента %1 обновлена клиентом.").arg(client->id()));
        }

        QVariantMap messageData;
        messageData[Keys::TIME_STAMP] = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        messageData[Keys::ID] = client->id();
        messageData[Keys::TYPE] = messageType;
        messageData[Keys::PAYLOAD] = payload.toVariantMap();
        m_dataBatch.append(messageData);
    } else {
        emit logMessage(QString("Получены данные от незарегистрированного клиента %1 типа %2").arg(client->descriptor()).arg(messageType));
    }
}

void DataProcessing::routeDataToClient(const QVariantMap &data) {
    quintptr dc = data[Keys::DESCRIPTOR].toULongLong();
    if (m_clients.contains(dc)) {
        ClientState &state = m_clients[dc];

        if (data[Keys::TYPE] == Keys::CONFIGURATION) {
            state.allowSending = data[Keys::ALLOW_SENDING].toBool();
            state.configuration = data[Keys::PAYLOAD].toMap();
            m_clientBatch.append(getClientDataMap(state));
            emit logMessage(QString("Новая конфигурация отправлена клиенту %1, отправка команд: %2").arg(data[Keys::ID].toString()).arg(data[Keys::ALLOW_SENDING].toBool() ? "разрешена" : "запрещена"));
        }

        QJsonDocument jsonDoc = QJsonDocument::fromVariant(data);
        QByteArray byteArrayData = jsonDoc.toJson(QJsonDocument::Compact);
        sendDataToClient(state.client, byteArrayData);
    } else {
        emit logMessage(QString("Не удалось сохранить конфигурацию: клиент %1 не найден.").arg(data[Keys::ID].toString()));
    }
}

void DataProcessing::sendDataToClient(IClient *client, const QByteArray &data) {
    if (!client) return;
    if (IServer *server = qobject_cast<IServer *>(client->parent())) {
        server->sendToClient(client, data);
    } else {
        emit logMessage("Не удалось найти сервер для отправки данных.");
    }
}

void DataProcessing::sendDataToAll(const QString &data) {
    int count = 0;
    QJsonObject jsonData;
    jsonData[Protocol::Keys::TYPE] = Protocol::MessageType::COMMAND;
    jsonData[Protocol::Keys::COMMAND] = data;

    QByteArray byteArray = QJsonDocument(jsonData).toJson(QJsonDocument::Compact);

    for (const auto &state : qAsConst(m_clients)) {
        if (state.allowSending && state.client && state.client->isConnected()) {
            sendDataToClient(state.client, byteArray);
            count++;
        }
    }
    emit logMessage(QString("Команда \"%1\" отправлена %2 клиентам.").arg(data).arg(count));
}
