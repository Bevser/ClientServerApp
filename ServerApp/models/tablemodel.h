/**
 * @file tablemodel.h
 * @brief Определяет базовую модель таблицы и ее наследников.
 */
#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QColor>
#include <QFont>
#include <QList>
#include <QTime>
#include <QVariantMap>

#include "core/appenums.h"
#include "core/sharedkeys.h"

/**
 * @class BaseTableModel
 * @brief Базовый класс для всех табличных моделей в приложении.
 *
 * Реализует основную логику работы с данными (добавление, удаление, сортировка)
 * и предоставляет общий интерфейс для QML.
 */
class BaseTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     */
    explicit BaseTableModel(QObject *parent = nullptr);

    // --- Переопределяемые виртуальные функции ---
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // --- Общие методы для управления данными ---
    /**
     * @brief Полностью заменяет данные в модели.
     * @param data Новый список данных.
     */
    void setData(const QList<QVariantMap> &data);
    /**
     * @brief Добавляет несколько строк в начало модели.
     * @param rowsData Список строк для добавления.
     */
    void addRows(const QList<QVariantMap> &rowsData);
    /**
     * @brief Добавляет одну строку в начало модели.
     * @param rowData Данные для новой строки.
     */
    void addRow(const QVariantMap &rowData);
    /**
     * @brief Обновляет данные в существующей строке.
     * @param row Индекс строки.
     * @param rowData Новые данные.
     */
    void updateRow(int row, const QVariantMap &rowData);
    /**
     * @brief Удаляет несколько строк.
     * @param row Начальная строка для удаления.
     * @param count Количество удаляемых строк.
     */
    void removeRows(int row, int count);
    /**
     * @brief Удаляет одну строку.
     * @param row Индекс строки для удаления.
     */
    void removeRow(int row);
    /**
     * @brief Очищает все данные из модели.
     */
    void clear();
    /**
     * @brief Сортирует модель по указанной колонке.
     * @param column Индекс колонки.
     * @param order Порядок сортировки.
     */
    void sortByColumn(int column, Qt::SortOrder order);

    // --- Методы для QML ---
    /**
     * @brief Возвращает данные строки в виде QVariantMap.
     */
    Q_INVOKABLE QVariantMap getRowData(int row) const;
    /// @brief Свойство с заголовками колонок.
    Q_PROPERTY(QStringList columnHeaders READ columnHeaders CONSTANT)
    /// @brief Свойство с относительной шириной колонок.
    Q_PROPERTY(QList<qreal> columnWidths READ columnWidths CONSTANT)

    QStringList columnHeaders() const { return m_headers; }
    QList<qreal> columnWidths() const { return m_columnWidths; }

signals:
    /**
     * @brief Сигнал для сброса иконки сортировки в UI.
     */
    void resetSorting();

protected:
    QStringList m_keys;
    QStringList m_headers;
    QList<qreal> m_columnWidths;
    QList<QVariantMap> m_data;
};

/**
 * @class ClientTableModel
 * @brief Модель для отображения таблицы клиентов.
 */
class ClientTableModel : public BaseTableModel {
    Q_OBJECT
public:
    /**
     * @enum Roles
     * @brief Кастомные роли для стилизации ячеек в QML.
     */
    enum Roles { StatusColorRole = Qt::UserRole + 1, IsBoldRole = Qt::UserRole + 2 };

    explicit ClientTableModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};

/**
 * @class DataTableModel
 * @brief Модель для отображения таблицы данных (сообщений) от клиентов.
 */
class DataTableModel : public BaseTableModel {
    Q_OBJECT
public:
    /**
     * @enum Roles
     * @brief Кастомные роли для стилизации ячеек в QML.
     */
    enum Roles { TypeColorRole = Qt::UserRole + 1 };

    explicit DataTableModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};

#endif // BASETABLEMODEL_H
