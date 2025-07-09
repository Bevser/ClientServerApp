/**
 * @file serverworker.h
 * @brief Определяет класс ServerWorker, который управляет серверами и обработкой данных в отдельном потоке.
 */
#ifndef SERVERWORKER_H
#define SERVERWORKER_H

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QThread>
#include <QTimer>

#include "core/dataprocessing.h"
#include "core/iserver.h"
#include "core/serverfactory.h"

/**
 * @class ServerWorker
 * @brief Основной рабочий класс, выполняющий все серверные операции в отдельном потоке.
 *
 * Этот класс отвечает за создание, запуск, остановку и удаление серверов.
 * Он также управляет пакетной отправкой данных, логов и обновлений статусов клиентов в основной поток (UI).
 */
class ServerWorker : public QObject {
    Q_OBJECT

    /// @brief Таймаут по умолчанию для таймера пакетной обработки (в миллисекундах).
    static constexpr int BATCH_TIMEOUT_MS           = 500;
    /// @brief Увеличенный таймаут для таймера при высокой нагрузке для снижения частоты обновлений.
    static constexpr int BATCH_TIMEOUT_SLOW_MODE_MS = 2000;

public:
    /**
     * @brief Конструктор класса ServerWorker.
     * @param parent Родительский объект QObject.
     */
    explicit ServerWorker(QObject *parent = nullptr);
    /**
     * @brief Деструктор класса ServerWorker.
     */
    ~ServerWorker();

    /**
     * @brief Возвращает указатель на экземпляр DataProcessing.
     * @return Указатель на DataProcessing.
     */
    DataProcessing *dataProcessing() const { return m_dataProcessing; }

public slots:
    /**
     * @brief Запускает сервер указанного типа на заданном порту.
     * @param type Тип сервера (TCP, UDP и т.д.).
     * @param port Порт для прослушивания.
     */
    void startServer(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Останавливает сервер указанного типа на заданном порту.
     * @param type Тип сервера.
     * @param port Порт сервера.
     */
    void stopServer(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Удаляет экземпляр сервера.
     * @param type Тип сервера.
     * @param port Порт сервера.
     */
    void deleteServer(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Отправляет данные всем подключенным и авторизованным клиентам.
     * @param data Данные для отправки.
     */
    void sendToAllClients(const QString &data);
    /**
     * @brief Обновляет конфигурацию для конкретного клиента.
     * @param config Карта с новой конфигурацией клиента.
     */
    void updateClientConfiguration(const QVariantMap &config);
    /**
     * @brief Удаляет клиентов, которые были отмечены как отключенные.
     */
    void removeDisconnectedClients();
    /**
     * @brief Очищает список всех клиентов.
     */
    void clearClients();

signals:
    // Сигналы для передачи в UI поток
    /**
     * @brief Сигнал о том, что сервер был запущен.
     */
    void serverStarted();
    /**
     * @brief Сигнал о том, что сервер был остановлен.
     */
    void serverStopped();
    /**
     * @brief Сигнал для обновления статуса сервера в UI.
     * @param type Тип сервера.
     * @param port Порт сервера.
     * @param status Новый статус сервера.
     * @param connections Количество активных подключений.
     */
    void serverStatusUpdate(AppEnums::ServerType type, quint16 port,
                            AppEnums::ServerStatus status, int connections);
    /**
     * @brief Сигнал, передающий пакет обновлений по клиентам.
     * @param clientBatch Список карт с данными клиентов.
     */
    void clientBatchReady(const QList<QVariantMap> &clientBatch);
    /**
     * @brief Сигнал, передающий пакет полученных от клиентов данных.
     * @param dataBatch Список карт с данными.
     */
    void dataBatchReady(const QList<QVariantMap> &dataBatch);
    /**
     * @brief Сигнал, передающий пакет логов.
     * @param logBatch Список строк логов.
     */
    void logBatchReady(const QStringList &logBatch);

private slots:
    /**
     * @brief Слот для обработки лог-сообщений от других объектов.
     * @param message Текст сообщения.
     */
    void handleLogMessage(const QString &message);
    /**
     * @brief Слот, вызываемый по таймауту таймера для пакетной обработки.
     * Собирает данные, обновления клиентов и логи и отправляет их в UI поток.
     */
    void handleBatchTimerTimeout();

private:
    /// @brief Таймер для пакетной отправки данных.
    QTimer *m_batchTimer;
    /// @brief Пакет для накопления сообщений лога.
    QStringList m_logBatch;

    /// @brief Указатель на объект обработки данных.
    DataProcessing *m_dataProcessing;
    /// @brief Хеш-таблица для хранения активных серверов.
    QHash<QPair<AppEnums::ServerType, quint16>, IServer *> m_servers;
};

#endif // SERVERWORKER_H
