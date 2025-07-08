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

    // Методы для генерации случайных данных (аналогично старому клиенту)
    QJsonObject generateNetworkMetrics();
    QJsonObject generateDeviceStatus();
    QJsonObject generateLog();

    // Переменные-члены класса
    QString m_host;
    quint16 m_port;
    IClient *m_client; // Указатель на интерфейс

    QTimer *m_reconnectTimer; // Таймер для попыток переподключения
    QTimer *m_dataSendTimer;  // Таймер для периодической отправки данных

    bool m_isStarted; // Флаг, разрешающий отправку данных
};

#endif // NEWCLIENTLOGIC_H
