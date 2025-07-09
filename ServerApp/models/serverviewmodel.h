/**
 * @file serverviewmodel.h
 * @brief Определяет класс ServerViewModel, который служит связующим звеном между UI (QML) и бэкендом.
 */
#ifndef SERVERVIEWMODEL_H
#define SERVERVIEWMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include "core/iserver.h"
#include "core/serverworker.h"
#include "models/tablemodel.h"
#include "models/serverlistmodel.h"

/**
 * @class ServerViewModel
 * @brief Класс ViewModel, управляющий логикой представления и взаимодействием с рабочим потоком.
 *
 * Этот класс предоставляет модели данных и свойства для QML, а также методы (Q_INVOKABLE),
 * которые можно вызывать из UI. Он делегирует все серверные операции объекту ServerWorker,
 * который выполняется в отдельном потоке.
 */
class ServerViewModel : public QObject {
    Q_OBJECT
    /// @brief Свойство для доступа к модели таблицы клиентов из QML.
    Q_PROPERTY(ClientTableModel *clientTableModel READ clientTableModel CONSTANT)
    /// @brief Свойство для доступа к модели таблицы данных из QML.
    Q_PROPERTY(DataTableModel *dataTableModel READ dataTableModel CONSTANT)
    /// @brief Свойство для доступа к модели списка серверов из QML.
    Q_PROPERTY(ServerListModel *serverListModel READ serverListModel CONSTANT)
    /// @brief Свойство для доступа к тексту лога из QML.
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)

    /// @brief Таймаут ожидания завершения рабочего потока (в миллисекундах).
    static constexpr int WORKER_THREAD_WAIT_TIMEOUT_MS = 5000;
    /// @brief Максимальное количество строк в таблице данных.
    static constexpr int MAX_DATA_TABLE_ROWS = 5000;
    /// @brief Количество строк, которое остается после обрезки таблицы данных.
    static constexpr int DATA_TABLE_TRIM_LENGTH = 2000;

public:
    /**
     * @brief Конструктор класса ServerViewModel.
     * @param parent Родительский объект QObject.
     */
    explicit ServerViewModel(QObject *parent = nullptr);
    /**
     * @brief Деструктор класса ServerViewModel.
     */
    ~ServerViewModel();

    // --- Геттеры для QML ---
    /**
     * @brief Возвращает указатель на модель таблицы клиентов.
     * @return Указатель на ClientTableModel.
     */
    ClientTableModel *clientTableModel() const;
    /**
     * @brief Возвращает указатель на модель таблицы данных.
     * @return Указатель на DataTableModel.
     */
    DataTableModel *dataTableModel() const;
    /**
     * @brief Возвращает указатель на модель списка серверов.
     * @return Указатель на ServerListModel.
     */
    ServerListModel *serverListModel() const;
    /**
     * @brief Возвращает текущий текст лога.
     * @return Строка с логами.
     */
    QString logText() const;

    // --- Методы, вызываемые из QML ---
    /**
     * @brief Добавляет сервер в список серверов.
     * @param type Тип сервера.
     * @param port Порт сервера.
     */
    Q_INVOKABLE void addServerToList(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Удаляет сервер из списка и посылает запрос на его удаление в рабочий поток.
     * @param type Тип сервера.
     * @param port Порт сервера.
     */
    Q_INVOKABLE void removeServerFromList(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Отправляет запрос на запуск сервера.
     * @param type Тип сервера.
     * @param port Порт сервера.
     */
    Q_INVOKABLE void startServer(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Отправляет запрос на остановку сервера.
     * @param type Тип сервера.
     * @param port Порт сервера.
     */
    Q_INVOKABLE void stopServer(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Отправляет команду "start" всем клиентам.
     */
    Q_INVOKABLE void startAllClients();
    /**
     * @brief Отправляет команду "stop" всем клиентам.
     */
    Q_INVOKABLE void stopAllClients();
    /**
     * @brief Отправляет запрос на удаление отключенных клиентов.
     */
    Q_INVOKABLE void removeDisconnectedClients();
    /**
     * @brief Отправляет запрос на обновление конфигурации клиента.
     * @param config Новая конфигурация.
     */
    Q_INVOKABLE void updateClientConfiguration(const QVariantMap &config);
    /**
     * @brief Сортирует таблицу клиентов по указанной колонке.
     * @param columnIndex Индекс колонки для сортировки.
     */
    Q_INVOKABLE void sortClients(int columnIndex);
    /**
     * @brief Сортирует таблицу данных по указанной колонке.
     * @param columnIndex Индекс колонки для сортировки.
     */
    Q_INVOKABLE void sortData(int columnIndex);
    /**
     * @brief Очищает текст лога.
     */
    Q_INVOKABLE void clearLog();
    /**
     * @brief Очищает таблицу данных.
     */
    Q_INVOKABLE void clearData();

public slots:
    // --- Слоты для обработки сигналов от рабочего потока ---
    /**
     * @brief Обрабатывает пакет обновлений по клиентам.
     * @param clientBatch Список с данными клиентов для обновления.
     */
    void handleClientBatchUpdate(const QList<QVariantMap> &clientBatch);
    /**
     * @brief Обрабатывает пакет полученных данных.
     * @param dataBatch Список с полученными данными.
     */
    void handleDataBatchReceived(const QList<QVariantMap> &dataBatch);
    /**
     * @brief Обрабатывает пакет логов.
     * @param logBatch Список строк лога.
     */
    void handleLogBatch(const QStringList &logBatch);
    /**
     * @brief Обрабатывает обновление статуса сервера.
     * @param type Тип сервера.
     * @param port Порт.
     * @param status Новый статус.
     * @param connections Количество подключений.
     */
    void handleServerStatusUpdate(AppEnums::ServerType type, quint16 port,
                                  AppEnums::ServerStatus status, int connections);
    /**
     * @brief Обрабатывает полную остановку всех серверов.
     */
    void handleServerStopped();

signals:
    /**
     * @brief Сигнал об изменении текста лога.
     */
    void logTextChanged();

    // --- Сигналы для отправки команд в рабочий поток ---
    /**
     * @brief Запрос на запуск сервера.
     */
    void startServerRequested(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Запрос на остановку сервера.
     */
    void stopServerRequested(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Запрос на удаление сервера.
     */
    void deleteServerRequested(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Запрос на отправку команды всем клиентам.
     */
    void sendToAllRequested(const QString &command);
    /**
     * @brief Запрос на обновление конфигурации клиента.
     */
    void updateClientConfigRequested(const QVariantMap &config);
    /**
     * @brief Запрос на удаление отключенных клиентов.
     */
    void removeDisconnectedRequested();
    /**
     * @brief Запрос на полную очистку списка клиентов.
     */
    void clearClientsRequested();

private:
    /**
     * @brief Настраивает и запускает рабочий поток.
     */
    void setupWorkerThread();

    // UI модели
    ClientTableModel *m_clientTableModel;
    DataTableModel *m_dataTableModel;
    ServerListModel *m_serverListModel;
    QString m_logText;

    // Переменные для хранения порядка сортировки
    Qt::SortOrder m_clientSortOrder;
    Qt::SortOrder m_dataSortOrder;

    // Рабочий поток
    QThread *m_workerThread;
    ServerWorker *m_serverWorker;
};

#endif // SERVERVIEWMODEL_H
