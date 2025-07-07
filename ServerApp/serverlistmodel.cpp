#include "serverlistmodel.h"

ServerListModel::ServerListModel(QObject *parent) : QAbstractListModel(parent) {}

int ServerListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_servers.count();
}

QVariant ServerListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_servers.count())
        return QVariant();

    const QVariantMap &server = m_servers[index.row()];
    auto status = static_cast<AppEnums::ServerStatus>(server.value("status").toInt());
    auto connections = server.value("connections").toInt();

    switch (role) {
    case TypeRole:
        return server.value("type");
    case TypeStrRole:
        return server.value("typeStr");
    case PortRole:
        return server.value("port");
    case StatusRole:
        return server.value("status");
    case ConnectionsRole:
        return server.value("connections");
    case StatusTextRole:
        return formatStatusText(status, connections);
    case StatusColorRole:
        return getStatusColor(status);
    case CanDeleteRole:
        return canPerformAction(status, "delete");
    case CanStartRole:
        return canPerformAction(status, "start");
    case CanStopRole:
        return canPerformAction(status, "stop");
    case ServerInfoRole: {
        auto type = static_cast<AppEnums::ServerType>(server.value("type").toInt());
        return AppEnums::typeToString(type);
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ServerListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[TypeRole]         = "type";
    roles[TypeStrRole]      = "typeStr";
    roles[PortRole]         = "port";
    roles[StatusRole]       = "status";
    roles[ConnectionsRole]  = "connections";
    roles[StatusTextRole]   = "statusText";
    roles[StatusColorRole]  = "statusColor";
    roles[CanDeleteRole]    = "canDelete";
    roles[CanStartRole]     = "canStart";
    roles[CanStopRole]      = "canStop";
    roles[ServerInfoRole]   = "serverInfo";
    return roles;
}

bool ServerListModel::addServer(AppEnums::ServerType type, quint16 port) {
    if (!canAddServer(type, port)) {
        return false;
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    QVariantMap server;
    server["type"] = (int)type;
    server["typeStr"] = AppEnums::typeToString(type);
    server["port"] = port;
    server["status"] = AppEnums::ServerStatus::STOPPED;
    server["connections"] = 0;
    m_servers.append(server);
    endInsertRows();
    return true;
}

void ServerListModel::removeServer(AppEnums::ServerType type, quint16 port) {
    for (int i = 0; i < m_servers.count(); ++i) {
        if (m_servers[i].value("port").toUInt() == port &&
            m_servers[i].value("type").toInt() == (int)type) {
            beginRemoveRows(QModelIndex(), i, i);
            m_servers.removeAt(i);
            endRemoveRows();
            return;
        }
    }
}

void ServerListModel::updateServerStatus(AppEnums::ServerType type, quint16 port,
                                         AppEnums::ServerStatus status, int connections) {
    for (int i = 0; i < m_servers.count(); ++i) {
        if (m_servers[i].value("port").toUInt() == port &&
            m_servers[i].value("type").toInt() == (int)type) {
            m_servers[i]["status"] = status;
            m_servers[i]["connections"] = connections;

            // Обновляем все связанные роли
            QVector<int> changedRoles = {
                StatusRole, ConnectionsRole, StatusTextRole,
                StatusColorRole, CanDeleteRole, CanStartRole, CanStopRole
            };
            emit dataChanged(index(i, 0), index(i, 0), changedRoles);
            return;
        }
    }
}

bool ServerListModel::canAddServer(AppEnums::ServerType type, quint16 port) const {
    // Проверка валидности порта
    if (!isPortValid(port)) {
        return false;
    }

    // Проверка на дубликаты
    return !isServerExists(type, port);
}

bool ServerListModel::isPortValid(quint16 port) const {
    return port >= 1024 && port <= 65535;
}

bool ServerListModel::isServerExists(AppEnums::ServerType type, quint16 port) const {
    for (const auto& server : qAsConst(m_servers)) {
        if (server.value("port").toUInt() == port &&
            server.value("type").toInt() == (int)type) {
            return true;
        }
    }
    return false;
}

QString ServerListModel::formatStatusText(AppEnums::ServerStatus status, int connections) const {
    QString statusStr = AppEnums::serverStatusToString(status);

    if (status == AppEnums::ServerStatus::RUNNING && connections > 0) {
        return QString("%1 (%2 подключений)").arg(statusStr).arg(connections);
    }

    return statusStr;
}

QString ServerListModel::getStatusColor(AppEnums::ServerStatus status) const {
    switch (status) {
    case AppEnums::ServerStatus::RUNNING:
        return "#4CAF50";  // Зеленый
    case AppEnums::ServerStatus::ERROR:
        return "#f44336";  // Красный
    case AppEnums::ServerStatus::STOPPED:
    default:
        return "#607D8B";  // Серый
    }
}

bool ServerListModel::canPerformAction(AppEnums::ServerStatus status, const QString& action) const {
    if (action == "delete") {
        return status != AppEnums::ServerStatus::RUNNING;
    } else if (action == "start") {
        return status == AppEnums::ServerStatus::STOPPED;
    } else if (action == "stop") {
        return status == AppEnums::ServerStatus::RUNNING;
    }

    return false;
}
