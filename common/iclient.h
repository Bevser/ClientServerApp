/**
 * @file iclient.h
 * @brief Определяет интерфейс IClient для всех типов клиентов.
 */
#ifndef ICLIENT_H
#define ICLIENT_H

#include <QObject>

/**
 * @class IClient
 * @brief Абстрактный базовый класс (интерфейс) для клиентских реализаций.
 *
 * Определяет общий набор функций и сигналов, которые должен предоставлять
 * любой класс-клиент, независимо от протокола.
 */
class IClient : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Виртуальный деструктор по умолчанию.
     */
    virtual ~IClient() = default;

    /**
     * @brief Возвращает уникальный дескриптор клиента (например, сокета).
     * @return Дескриптор.
     */
    virtual quintptr descriptor() const = 0;
    /**
     * @brief Возвращает IP-адрес клиента.
     * @return Адрес в виде строки.
     */
    virtual QString address() const = 0;
    /**
     * @brief Возвращает порт клиента.
     * @return Порт.
     */
    virtual quint16 port() const = 0;
    /**
     * @brief Возвращает строковый идентификатор клиента.
     * @return ID клиента.
     */
    virtual QString id() const = 0;
    /**
     * @brief Проверяет, подключен ли клиент.
     * @return true, если подключен, иначе false.
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief Устанавливает строковый идентификатор клиента.
     * @param id Новый ID.
     */
    virtual void setId(const QString &id) = 0;

    /**
     * @brief Отправляет данные клиенту.
     * @param data Данные для отправки.
     */
    virtual void sendData(const QByteArray &data) = 0;

    /**
     * @brief Подключается к хосту.
     * @param host Адрес хоста.
     * @param port Порт хоста.
     */
    virtual void connectToHost(const QString &host, quint16 port) = 0;
    /**
     * @brief Отключается от хоста.
     */
    virtual void disconnect() = 0;

protected slots:
    /**
     * @brief Внутренний слот для обработки события подключения.
     */
    virtual void handleConnected() = 0;
    /**
     * @brief Внутренний слот для обработки события отключения.
     */
    virtual void handleDisconnected() = 0;
    /**
     * @brief Внутренний слот для обработки готовности к чтению данных.
     */
    virtual void handleReadyRead() = 0;

signals:
    /**
     * @brief Сигнал, испускаемый при успешном подключении.
     */
    void connected();
    /**
     * @brief Сигнал, испускаемый при отключении.
     */
    void disconnected();
    /**
     * @brief Сигнал, испускаемый при получении данных.
     * @param data Полученные данные.
     */
    void dataReceived(const QByteArray &data);
    /**
     * @brief Сигнал, испускаемый при возникновении ошибки.
     * @param message Сообщение об ошибке.
     */
    void errorOccurred(const QString &message);

protected:
    /**
     * @brief Защищенный конструктор, чтобы предотвратить прямое создание IClient.
     * @param parent Родительский объект QObject.
     */
    explicit IClient(QObject *parent = nullptr) : QObject(parent) {}
};

#endif // ICLIENT_H
