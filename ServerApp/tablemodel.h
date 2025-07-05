#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QVariantMap>
#include <QTime>
#include <QFont>
#include <QColor>

#include "sharedkeys.h"
#include "appenums.h"



class BaseTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit BaseTableModel(QObject *parent = nullptr);

    // --- Переопределяемые виртуальные функции ---
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // --- Общие методы для управления данными ---
    void setData(const QList<QVariantMap>& data);
    void addRows(const QList<QVariantMap>& rowsData);
    void addRow(const QVariantMap& rowData);
    void updateRow(int row, const QVariantMap& rowData);
    void removeRows(int row, int count);
    void removeRow(int row);
    void clear();
    void sortByColumn(int column, Qt::SortOrder order);

    // --- Методы для QML ---
    Q_INVOKABLE QVariantMap getRowData(int row) const;
    Q_PROPERTY(QStringList columnHeaders READ columnHeaders CONSTANT)
    Q_PROPERTY(QList<qreal> columnWidths READ columnWidths CONSTANT)

    QStringList columnHeaders() const { return m_headers; }
    QList<qreal> columnWidths() const { return m_columnWidths; }

signals:
    void resetSorting();

protected:
    // Защищенные члены доступны для дочерних классов
    QStringList m_keys;
    QStringList m_headers;
    QList<qreal> m_columnWidths;
    QList<QVariantMap> m_data;
};


class ClientTableModel : public BaseTableModel
{
    Q_OBJECT

public:
    // Определяем кастомные роли для QML
    enum Roles {
        StatusColorRole = Qt::UserRole + 1,
        IsBoldRole = Qt::UserRole + 2
    };

    explicit ClientTableModel(QObject *parent = nullptr);

    // Переопределяем ключевые виртуальные функции
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};

class DataTableModel : public BaseTableModel
{
    Q_OBJECT

public:
    // Кастомные роли для стилизации
    enum Roles {
        TypeColorRole = Qt::UserRole + 1
    };

    explicit DataTableModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};

#endif // BASETABLEMODEL_H
