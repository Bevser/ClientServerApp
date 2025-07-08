#include "tablemodel.h"

BaseTableModel::BaseTableModel(QObject *parent) : QAbstractTableModel(parent) {}

int BaseTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_data.size();
}

int BaseTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_keys.size();
}

QVariant BaseTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_data.size() ||
        index.column() >= m_keys.size()) {
        return QVariant();
    }
    // Базовая реализация возвращает только сырые данные
    if (role == Qt::DisplayRole) {
        return m_data.at(index.row()).value(m_keys.at(index.column()));
    }
    return QVariant();
}

QVariant BaseTableModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole &&
        section < m_headers.size()) {
        return m_headers.at(section);
    }
    return QVariant();
}

QHash<int, QByteArray> BaseTableModel::roleNames() const {
    // Базовые роли, которые понимает TableView в QML
    return {{Qt::DisplayRole, "display"}};
}

void BaseTableModel::setData(const QList<QVariantMap> &data) {
    beginResetModel();
    m_data = data;
    endResetModel();
    emit resetSorting();
}

void BaseTableModel::addRows(const QList<QVariantMap> &rowsData) {
    if (rowsData.isEmpty())
        return;
    beginInsertRows(QModelIndex(), 0, rowsData.size() - 1);
    for (const auto &row : rowsData) {
        m_data.prepend(row);
    }
    endInsertRows();
    emit resetSorting();
}

void BaseTableModel::addRow(const QVariantMap &rowData) {
    beginInsertRows(QModelIndex(), 0, 0);
    m_data.prepend(rowData);
    endInsertRows();
}

void BaseTableModel::updateRow(int row, const QVariantMap &rowData) {
    if (row >= 0 && row < m_data.size()) {
        m_data[row] = rowData;
        emit dataChanged(index(row, 0), index(row, m_keys.size() - 1));
    }
}

void BaseTableModel::removeRows(int row, int count) {
    if (row < 0 || row >= m_data.size() || count <= 0)
        return;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    m_data.erase(m_data.begin() + row, m_data.begin() + row + count);
    endRemoveRows();
}

void BaseTableModel::removeRow(int row) {
    if (row < 0 || row >= m_data.size())
        return;
    beginRemoveRows(QModelIndex(), row, row);
    m_data.removeAt(row);
    endRemoveRows();
}

void BaseTableModel::clear() {
    beginResetModel();
    m_data.clear();
    endResetModel();
    emit resetSorting();
}

void BaseTableModel::sortByColumn(int column, Qt::SortOrder order) {
    if (column < 0 || column >= m_keys.size())
        return;
    const QString &key = m_keys.at(column);

    beginResetModel();
    std::sort(
        m_data.begin(), m_data.end(),
        [key, order](const QVariantMap &a, const QVariantMap &b) {
        QVariant valueA = a[key];
        QVariant valueB = b[key];

        if (key == Keys::TIME_STAMP) {
            QTime timeA = QTime::fromString(valueA.toString(), "hh:mm:ss.zzz");
            QTime timeB = QTime::fromString(valueB.toString(), "hh:mm:ss.zzz");
            return (order == Qt::AscendingOrder) ? (timeA < timeB)
                                                 : (timeA > timeB);
        }

        if (key == Keys::ID) {
            auto parseId = [](const QString &id) -> std::pair<QString, int> {
                int underscorePos = id.lastIndexOf('_');
                if (underscorePos == -1) {
                    // Если нет подчеркивания, возвращаем весь текст и 0
                    return {id, 0};
                }

                QString textPart = id.left(underscorePos);
                QString numberPart = id.mid(underscorePos + 1);

                bool ok;
                int number = numberPart.toInt(&ok);
                if (!ok) {
                    // Если не удалось преобразовать в число, возвращаем 0
                    number = 0;
                }

                return {textPart, number};
            };

            auto [textA, numA] = parseId(valueA.toString());
            auto [textB, numB] = parseId(valueB.toString());

            // Сначала сравниваем текстовую часть
            int textCompare = QString::compare(textA, textB, Qt::CaseInsensitive);
            if (textCompare != 0) {
            return (order == Qt::AscendingOrder) ? (textCompare < 0)
                                                     : (textCompare > 0);
            }

            // Если текстовые части равны, сравниваем числовые части
            return (order == Qt::AscendingOrder) ? (numA < numB) : (numA > numB);
        }
        // Обобщенная сортировка по строкам
        return (order == Qt::AscendingOrder)
            ? (valueA.toString().toLower() < valueB.toString().toLower())
                   : (valueA.toString().toLower() >
                      valueB.toString().toLower());
        });
    endResetModel();
}

QVariantMap BaseTableModel::getRowData(int row) const {
    if (row >= 0 && row < m_data.size()) {
        return m_data.at(row);
    }
    return QVariantMap();
}

ClientTableModel::ClientTableModel(QObject *parent) : BaseTableModel(parent) {
    m_keys          = {Keys::ID,        Keys::ADDRESS,  Keys::STATUS,   Keys::ALLOW_SENDING};
    m_headers       = {"ID Клиента",    "Адрес",        "Статус",       "Отправка"};
    m_columnWidths  = {0.25,            0.30,           0.25,           0.20};
}

QHash<int, QByteArray> ClientTableModel::roleNames() const {
    QHash<int, QByteArray> roles = BaseTableModel::roleNames();
    roles[StatusColorRole] = "statusColor";
    roles[IsBoldRole] = "isBold";
    return roles;
}

QVariant ClientTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    const QVariantMap &rowData = m_data.at(index.row());
    const QString &key = m_keys.at(index.column());
    QVariant value = rowData.value(key);

    switch (role) {
    case Qt::DisplayRole: {
        if (key == Keys::STATUS) {
            return AppEnums::clientStatusToString(
                static_cast<AppEnums::ClientStatus>(value.toInt()));
        }
        if (key == Keys::ALLOW_SENDING) {
            return value.toBool() ? "Да" : "Нет";
        }
        return value.toString();
    }

    case StatusColorRole: {
        if (key == Keys::STATUS) {
            auto status = static_cast<AppEnums::ClientStatus>(value.toInt());
            if (status == AppEnums::CONNECTED)
                return QColor("#4CAF50");
            if (status == AppEnums::AUTHORIZING)
                return QColor("#FF9800");
            if (status == AppEnums::DISCONNECTED)
                return QColor("#f44336");
        }
        if (key == Keys::ALLOW_SENDING) {
            return value.toBool() ? QColor("#4CAF50") : QColor("#f44336");
        }
        return QColor("#424242"); // Цвет по умолчанию
    }

    case IsBoldRole: {
        if (key == Keys::STATUS || key == Keys::ALLOW_SENDING) {
            return true;
        }
        return false;
    }
    }

    return QVariant();
}

DataTableModel::DataTableModel(QObject *parent) : BaseTableModel(parent) {
    m_keys          = {Keys::TIME_STAMP,    Keys::ID,   Keys::TYPE, Keys::PAYLOAD};
    m_headers       = {"Время",             "ID",       "Тип",      "Сообщение"};
    m_columnWidths  = {0.15,                0.20,       0.15,       0.50};
}

QHash<int, QByteArray> DataTableModel::roleNames() const {
    QHash<int, QByteArray> roles = BaseTableModel::roleNames();
    roles[TypeColorRole] = "typeColor";
    return roles;
}

QVariant DataTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    const QVariantMap &rowData = m_data.at(index.row());
    const QString &key = m_keys.at(index.column());
    QVariant value = rowData.value(key);

    switch (role) {
    case Qt::DisplayRole: {
        if (key == Keys::PAYLOAD && value.typeId() == QMetaType::QVariantMap) {
            QVariantMap payloadMap = value.toMap();
            QStringList items;
            for (auto it = payloadMap.constBegin(); it != payloadMap.constEnd();
                 ++it) {
                items << QString("%1: %2").arg(it.key()).arg(it.value().toString());
            }
            return items.join(", ");
        }
        return value.toString();
    }

    case TypeColorRole: {
        if (key == Keys::TYPE) {
            QString typeStr = value.toString();
            if (typeStr == "NetworkMetrics")
                return QColor("#0891b2");
            if (typeStr == "DeviceStatus")
                return QColor("#2196F3");
            if (typeStr == "Log")
                return QColor("#8f4cf6");
        }
        return QColor("#424242"); // Цвет по умолчанию
    }
    }

    return QVariant();
}
