/**
 * @file clientlogic.h
 * @brief Определяет класс ClientLogic, который инкапсулирует всю логику работы клиента.
 */
#ifndef NEWCLIENTLOGIC_H
#define NEWCLIENTLOGIC_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>
#include <QRandomGenerator>
#include <QTcpSocket>
#include <QTimer>

#include "../common/protocol.h"  // Общий протокол обмена данными
#include "../common/tcpclient.h" // Интерфейс клиента
#include "clientprotocol.h"      // Внутренний протокол клиента

/**
 * @struct ClientConfiguration
 * @brief Структура для хранения и управления конфигурацией клиента.
 *
 * Хранит пороговые значения для различных метрик и предоставляет
 * методы для сериализации/десериализации в/из JSON.
 */
struct ClientConfiguration {
    double maxCpuTemp;      ///< Максимальная температура процессора
    double maxCpuUsage;     ///< Максимальная загрузка процессора (%)
    double maxMemoryUsage;  ///< Максимальная загрузка памяти (%)
    double maxBandWidth;    ///< Максимальная пропускная способность
    double maxLatency;      ///< Максимальная задержка
    double maxPacketLoss;   ///< Максимальная потеря пакетов

    /**
     * @brief Конструктор по умолчанию. Инициализирует поля случайными значениями.
     */
    ClientConfiguration()
        : maxCpuTemp(QRandomGenerator::global()->bounded(80, 95))
        , maxCpuUsage(QRandomGenerator::global()->bounded(80, 95))
        , maxMemoryUsage(QRandomGenerator::global()->bounded(80, 95))
        , maxBandWidth(1000.0)
        , maxLatency(100.0)
        , maxPacketLoss(5.0)
    {}

    /**
     * @brief Загружает конфигурацию из JSON-объекта.
     * @param json JSON-объект с новыми значениями.
     */
    void loadFromJson(const QJsonObject &json);

    /**
     * @brief Сохраняет текущую конфигурацию в JSON-объект.
     * @return QJsonObject с конфигурацией.
     */
    QJsonObject toJson() const;
};

/**
 * @class ClientLogic
 * @brief Основной класс, управляющий поведением клиента.
 *
 * Отвечает за подключение к серверу, автоматическое переподключение,
 * отправку регистрационных данных, периодическую отправку телеметрии,
 * обработку команд и конфигураций от сервера.
 */
class ClientLogic : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Конструктор класса.
     * @param host Адрес сервера.
     * @param port Порт сервера.
     * @param parent Родительский объект QObject.
     */
    explicit ClientLogic(const QString &host, quint16 port, QObject *parent = nullptr);
    /**
     * @brief Деструктор.
     */
    ~ClientLogic();

    /**
     * @brief Запускает логику клиента, инициируя первое подключение.
     */
    void start();

private slots:
    // --- Слоты для обработки сигналов от IClient ---
    /**
     * @brief Обрабатывает успешное подключение к серверу.
     */
    void handleClientConnected();
    /**
     * @brief Обрабатывает отключение от сервера и запускает логику переподключения.
     */
    void handleClientDisconnected();
    /**
     * @brief Обрабатывает получение данных от сервера.
     * @param data Полученные данные.
     */
    void handleDataReceived(const QByteArray &data);
    /**
     * @brief Обрабатывает ошибки сокета.
     * @param error Текст ошибки.
     */
    void handleClientError(const QString &error);

    // --- Слоты для таймеров ---
    /**
     * @brief Выполняет попытку подключения к серверу (вызывается по таймеру).
     */
    void connectToServer();
    /**
     * @brief Отправляет периодические данные на сервер (вызывается по таймеру).
     */
    void sendPeriodicData();

private:
    /**
     * @brief Отправляет на сервер запрос на регистрацию.
     */
    void sendRegistrationRequest();
    /**
     * @brief Отправляет JSON-объект на сервер.
     * @param json Объект для отправки.
     */
    void sendJson(const QJsonObject &json);
    /**
     * @brief Устанавливает соединения сигналов и слотов для объекта клиента.
     */
    void setupClientConnections();

    // --- Методы для генерации случайных данных ---
    /**
     * @brief Генерирует JSON-объект с метриками сети.
     */
    QJsonObject generateNetworkMetrics();
    /**
     * @brief Генерирует JSON-объект со статусом устройства.
     */
    QJsonObject generateDeviceStatus();
    /**
     * @brief Генерирует JSON-объект с лог-сообщением.
     */
    QJsonObject generateLog();

    // --- Методы для проверки пороговых значений ---
    /**
     * @brief Проверяет данные на превышение пороговых значений из конфигурации.
     * @param data JSON-объект с данными (может быть модифицирован).
     * @return true, если было обнаружено превышение.
     */
    bool checkThresholds(QJsonObject &data);
    /**
     * @brief Проверяет метрики сети.
     * @param payload Полезная нагрузка сообщения.
     * @param criticalMessages Список для добавления сообщений о превышениях.
     */
    void checkNetworkMetrics(const QJsonObject &payload, QStringList &criticalMessages);
    /**
     * @brief Проверяет статус устройства.
     * @param payload Полезная нагрузка сообщения.
     * @param criticalMessages Список для добавления сообщений о превышениях.
     */
    void checkDeviceStatus(const QJsonObject &payload, QStringList &criticalMessages);

    // --- Переменные-члены класса ---
    QString m_host;         ///< Адрес хоста сервера.
    quint16 m_port;         ///< Порт сервера.
    IClient *m_client;      ///< Указатель на интерфейс клиента.

    QTimer *m_reconnectTimer; ///< Таймер для попыток переподключения.
    QTimer *m_dataSendTimer;  ///< Таймер для периодической отправки данных.

    bool m_isStarted;       ///< Флаг, разрешающий отправку данных (управляется командами с сервера).

    ClientConfiguration m_config; ///< Текущая конфигурация клиента.
};

#endif // NEWCLIENTLOGIC_H
