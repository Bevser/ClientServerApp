/**
 * @file serverlistmodel.h
 * @brief Определяет модель списка серверов для QML.
 */
#ifndef SERVERLISTMODEL_H
#define SERVERLISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QVariantMap>

#include "core/appenums.h"
#include "../common/protocol.h"

/**
 * @class ServerListModel
 * @brief Модель данных, представляющая список серверов в UI.
 *
 * Эта модель используется в QML для отображения списка добавленных серверов,
 * их статусов и для управления ими.
 */
class ServerListModel : public QAbstractListModel {
    Q_OBJECT

public:
    /**
     * @enum ServerRoles
     * @brief Роли данных для доступа к элементам модели из QML.
     */
    enum ServerRoles {
        TypeRole = Qt::UserRole + 1,
        TypeStrRole,
        PortRole,
        StatusRole,
        ConnectionsRole,
        StatusTextRole,  // Готовая строка статуса с подключениями
        StatusColorRole, // Цвет для индикации статуса
        CanDeleteRole,   // Можно ли удалить сервер
        CanStartRole,    // Можно ли запустить сервер
        CanStopRole,     // Можно ли остановить сервер
        ServerInfoRole   // Полная информация о сервере
    };

    explicit ServerListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    /**
     * @brief Добавляет новый сервер в модель.
     * @param type Тип сервера.
     * @param port Порт сервера.
     * @return true, если сервер был успешно добавлен, иначе false.
     */
    bool addServer(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Удаляет сервер из модели.
     * @param type Тип сервера.
     * @param port Порт сервера.
     */
    void removeServer(AppEnums::ServerType type, quint16 port);
    /**
     * @brief Обновляет статус существующего сервера в модели.
     * @param type Тип сервера.
     * @param port Порт сервера.
     * @param status Новый статус.
     * @param connections Количество подключений.
     */
    void updateServerStatus(AppEnums::ServerType type, quint16 port,
                            AppEnums::ServerStatus status, int connections);

    // --- Методы для валидации, вызываемые из QML ---
    /**
     * @brief Проверяет, можно ли добавить такой сервер.
     */
    Q_INVOKABLE bool canAddServer(AppEnums::ServerType type, quint16 port) const;
    /**
     * @brief Проверяет валидность порта.
     */
    Q_INVOKABLE bool isPortValid(quint16 port) const;
    /**
     * @brief Проверяет, существует ли уже такой сервер в списке.
     */
    Q_INVOKABLE bool isServerExists(AppEnums::ServerType type, quint16 port) const;

private:
    /// @brief Список серверов, хранящихся в модели.
    QList<QVariantMap> m_servers;

    // --- Вспомогательные методы для вычисления производных значений ---
    QString formatStatusText(AppEnums::ServerStatus status, int connections) const;
    QString getStatusColor(AppEnums::ServerStatus status) const;
    bool canPerformAction(AppEnums::ServerStatus status, const QString &action) const;
};

#endif // SERVERLISTMODEL_H
