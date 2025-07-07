#ifndef SERVERLISTMODEL_H
#define SERVERLISTMODEL_H

#include <QAbstractListModel>
#include <QVariantMap>
#include <QList>
#include "appenums.h"

class ServerListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ServerRoles {
        TypeRole = Qt::UserRole + 1,
        TypeStrRole,
        PortRole,
        StatusRole,
        ConnectionsRole,
        StatusTextRole,      // готовая строка статуса с подключениями
        StatusColorRole,     // цвет для индикации статуса
        CanDeleteRole,       // можно ли удалить сервер
        CanStartRole,        // можно ли запустить сервер
        CanStopRole,         // можно ли остановить сервер
        ServerInfoRole       // полная информация о сервере
    };

    explicit ServerListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    bool addServer(AppEnums::ServerType type, quint16 port);
    void removeServer(AppEnums::ServerType type, quint16 port);
    void updateServerStatus(AppEnums::ServerType type, quint16 port,
                            AppEnums::ServerStatus status, int connections);

    // Методы для валидации
    Q_INVOKABLE bool canAddServer(AppEnums::ServerType type, quint16 port) const;
    Q_INVOKABLE bool isPortValid(quint16 port) const;
    Q_INVOKABLE bool isServerExists(AppEnums::ServerType type, quint16 port) const;

private:
    QList<QVariantMap> m_servers;

    // Вспомогательные методы для вычисления производных значений
    QString formatStatusText(AppEnums::ServerStatus status, int connections) const;
    QString getStatusColor(AppEnums::ServerStatus status) const;
    bool canPerformAction(AppEnums::ServerStatus status, const QString& action) const;
};

#endif // SERVERLISTMODEL_H
