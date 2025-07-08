#ifndef NEWCLIENTLOGIC_H
#define NEWCLIENTLOGIC_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>
#include <QRandomGenerator>
#include <QTcpSocket>
#include <QTimer>

#include "../common/protocol.h"  // Протокол обмена данными
#include "../common/tcpclient.h" // Интерфейс клиента
#include "clientprotocol.h"      // Внутренний протокол клиента

// Структура для хранения конфигурации
struct ClientConfiguration {
    double maxCpuTemp;      // Максимальная температура процессора
    double maxCpuUsage;     // Максимальная загрузка процессора (%)
    double maxMemoryUsage;  // Максимальная загрузка памяти (%)
    double maxBandWidth;    // Максимальная пропускная способность
    double maxLatency;      // Максимальная задержка
    double maxPacketLoss;   // Максимальная потеря пакетов

    // Конструктор по умолчанию с начальными значениями
    ClientConfiguration()
        : maxCpuTemp(QRandomGenerator::global()->bounded(70, 95))
        , maxCpuUsage(QRandomGenerator::global()->bounded(70, 95))
        , maxMemoryUsage(QRandomGenerator::global()->bounded(70, 95))
        , maxBandWidth(1000.0)
        , maxLatency(100.0)
        , maxPacketLoss(5.0)
    {}

    // Метод для загрузки конфигурации из JSON
    void loadFromJson(const QJsonObject &json) {
        if (json.contains(Protocol::Keys::MAX_CPU_TEMP))
            maxCpuTemp = json[Protocol::Keys::MAX_CPU_TEMP].toString().toDouble();
        if (json.contains(Protocol::Keys::MAX_CPU_USAGE))
            maxCpuUsage = json[Protocol::Keys::MAX_CPU_USAGE].toString().toDouble();
        if (json.contains(Protocol::Keys::MAX_MEMORY_USAGE))
            maxMemoryUsage = json[Protocol::Keys::MAX_MEMORY_USAGE].toString().toDouble();
        if (json.contains(Protocol::Keys::MAX_BAND_WIDTH))
            maxBandWidth = json[Protocol::Keys::MAX_BAND_WIDTH].toString().toDouble();
        if (json.contains(Protocol::Keys::MAX_LATENCY))
            maxLatency = json[Protocol::Keys::MAX_LATENCY].toString().toDouble();
        if (json.contains(Protocol::Keys::MAX_PACKET_LOSS))
            maxPacketLoss = json[Protocol::Keys::MAX_PACKET_LOSS].toString().toDouble();
    }

    // Метод для сохранения конфигурации в JSON
    QJsonObject toJson() const {
        QJsonObject json;
        json[Protocol::Keys::MAX_CPU_TEMP] = QString::number(maxCpuTemp, 'f', 2);
        json[Protocol::Keys::MAX_CPU_USAGE] = QString::number(maxCpuUsage, 'f', 2);
        json[Protocol::Keys::MAX_MEMORY_USAGE] = QString::number(maxMemoryUsage, 'f', 2);
        json[Protocol::Keys::MAX_BAND_WIDTH] = QString::number(maxBandWidth, 'f', 2);
        json[Protocol::Keys::MAX_LATENCY] = QString::number(maxLatency, 'f', 2);
        json[Protocol::Keys::MAX_PACKET_LOSS] = QString::number(maxPacketLoss, 'f', 2);
        return json;
    }
};

class ClientLogic : public QObject {
    Q_OBJECT

public:
    // Конструктор принимает адрес, порт и родительский объект
    explicit ClientLogic(const QString &host, quint16 port,
                         QObject *parent = nullptr);
    // Деструктор для корректного освобождения ресурсов
    ~ClientLogic();

    // Метод для запуска клиента
    void start();

private slots:
    // Слоты для обработки сигналов от IClient
    void handleClientConnected();
    void handleClientDisconnected();
    void handleDataReceived(const QByteArray &data);
    void handleClientError(const QString &error);

    // Слоты для таймеров
    void connectToServer();  // Слот для таймера переподключения
    void sendPeriodicData(); // Слот для таймера отправки данных

private:
    // Приватные методы для инкапсуляции логики
    void sendRegistrationRequest();
    void sendJson(const QJsonObject &json);
    void setupClientConnections();
    void sendCriticalAlert(const QString &parameter, double value, double threshold);

    // Методы для генерации случайных данных
    QJsonObject generateNetworkMetrics();
    QJsonObject generateDeviceStatus();
    QJsonObject generateLog();

    // Методы для проверки пороговых значений
    bool checkThresholds(QJsonObject &data);
    void checkNetworkMetrics(const QJsonObject &payload, QStringList &criticalMessages);
    void checkDeviceStatus(const QJsonObject &payload, QStringList &criticalMessages);

    // Переменные-члены класса
    QString m_host;
    quint16 m_port;
    IClient *m_client; // Указатель на интерфейс

    QTimer *m_reconnectTimer; // Таймер для попыток переподключения
    QTimer *m_dataSendTimer;  // Таймер для периодической отправки данных

    bool m_isStarted; // Флаг, разрешающий отправку данных

    ClientConfiguration m_config; // Конфигурация клиента
};

#endif // NEWCLIENTLOGIC_H
