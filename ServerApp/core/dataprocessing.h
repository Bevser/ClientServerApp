/**
 * @file dataprocessing.h
 * @brief Определяет класс DataProcessing для обработки данных от клиентов.
 */
#ifndef DATAPROCESSING_H
#define DATAPROCESSING_H

#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>
#include <QSet>
#include <QVariantMap>

#include "../common/iclient.h"
#include "core/appenums.h"
#include "core/iserver.h"
#include "core/sharedkeys.h"

/**
 * @class DataProcessing
 * @brief Класс, отвечающий за централизованную обработку данных.
 *
 * Этот класс управляет состояниями всех клиентов, обрабатывает входящие
 * сообщения, регистрирует клиентов, парсит JSON-данные и формирует
 * пакеты данных (batch) для отправки в UI-поток.
 */
class DataProcessing : public QObject {
    Q_OBJECT

    /**
     * @struct ClientState
     * @brief Структура для хранения полного состояния клиента.
     */
    struct ClientState {
        IClient *client = nullptr; ///< Указатель на объект клиента.
        AppEnums::ClientStatus status = AppEnums::DISCONNECTED; ///< Текущий статус клиента.
        bool allowSending = false; ///< Флаг, разрешающий отправку команд клиенту.
        QVariantMap configuration; ///< Конфигурация, связанная с клиентом.
    };

public:
    /**
     * @brief Конструктор класса DataProcessing.
     * @param parent Родительский объект QObject.
     */
    explicit DataProcessing(QObject *parent = nullptr);
    /**
     * @brief Деструктор класса DataProcessing.
     */
    ~DataProcessing();

    /**
     * @brief Добавляет сервер для обработки его событий.
     * @param server Указатель на IServer.
     */
    void addServer(IServer *server);

    /**
     * @brief Забирает накопленный пакет обновлений по клиентам.
     * @return Список карт с данными об изменениях клиентов.
     */
    QList<QVariantMap> takeClientUpdatesBatch();
    /**
     * @brief Забирает накопленный пакет входящих данных от клиентов.
     * @return Список карт с данными.
     */
    QList<QVariantMap> takeDataBatch();

public slots:
    /**
     * @brief Отправляет данные всем авторизованным клиентам.
     * @param data Данные для отправки.
     */
    void sendDataToAll(const QString &data);

    /**
     * @brief Направляет данные (например, конфигурацию) конкретному клиенту.
     * @param data Карта с данными, содержащая дескриптор и полезную нагрузку.
     */
    void routeDataToClient(const QVariantMap &data);
    /**
     * @brief Отправляет данные конкретному клиенту.
     * @param client Указатель на клиента.
     * @param data Данные в виде QByteArray.
     */
    void sendDataToClient(IClient *client, const QByteArray &data);
    /**
     * @brief Удаляет клиентов, помеченных как DISCONNECTED.
     */
    void removeDisconnectedClients();
    /**
     * @brief Удаляет клиента.
     * @param state Указатель на состояние клиента.
     */
    void removeClient(ClientState &state);
    /**
     * @brief Очищает все списки клиентов.
     */
    void clearClients();

private slots:
    /**
     * @brief Обрабатывает подключение нового клиента.
     * @param client Указатель на подключенного клиента.
     */
    void handleClientConnected(IClient *client);
    /**
     * @brief Обрабатывает отключение клиента.
     * @param client Указатель на отключившегося клиента.
     */
    void handleClientDisconnected(IClient *client);
    /**
     * @brief Обрабатывает получение данных от клиента.
     * @param client Клиент-отправитель.
     * @param data Полученные данные.
     */
    void handleDataReceived(IClient *client, const QByteArray &data);

signals:
    /**
     * @brief Сигнал с обновленной информацией о клиенте.
     * @param data Карта с данными клиента.
     */
    void clientUpdate(const QVariantMap &data);
    /**
     * @brief Сигнал о получении данных от клиента.
     * @param data Карта с данными.
     */
    void dataReceived(const QVariantMap &data);
    /**
     * @brief Сигнал для логирования сообщения.
     * @param message Текст сообщения.
     */
    void logMessage(const QString &message);

private:
    /**
     * @brief Парсит данные от клиента как JSON.
     * @param client Клиент-отправитель.
     * @param data Полученные данные.
     */
    void parseJsonData(IClient *client, const QByteArray &data);
    /**
     * @brief Регистрирует клиента в системе.
     * @param client Указатель на клиента.
     * @param id Запрашиваемый ID клиента.
     * @param payload Полезная нагрузка из регистрационного сообщения.
     */
    void registerClient(IClient *client, const QString &id,
                        const QJsonObject &payload);
    /**
     * @brief Формирует QVariantMap с данными о состоянии клиента.
     * @param state Состояние клиента.
     * @return Карта с данными клиента.
     */
    QVariantMap getClientDataMap(const ClientState &state);

    /// @brief Пакет для обновлений информации о клиентах.
    QList<QVariantMap> m_clientBatch;
    /// @brief Пакет для входящих данных от клиентов.
    QList<QVariantMap> m_dataBatch;

    /// @brief Хеш-таблица для хранения состояний клиентов по дескриптору.
    QHash<quintptr, ClientState> m_clients;
    /// @brief Множество для хранения уже используемых ID клиентов.
    QSet<QString> m_usedClientIds;
};

#endif // DATAPROCESSING_H
