import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Универсальный компонент таблицы с настраиваемыми заголовками и данными
Item {
    id: universalTable

    // Публичные свойства
    property string title: "Таблица"
    property var columnHeaders: []          // Массив объектов с заголовками: [{text: "Заголовок", width: 0.33}, ...]
    property var tableModel: null           // Модель данных
    property var onSort: null               // Callback функция для сортировки: function(columnIndex) {}
    property var onCellClicked: null        // Callback для клика по ячейке: function(row, column, value) {}
    property var cellStyler: null           // Функция для стилизации ячеек: function(row, column, value) { return {color: "black", bold: false} }

    // Цветовые свойства
    property color headerColor: "#37474f"
    property color headerHoverColor: "#455a64"
    property color headerBorderColor: "#263238"
    property color headerTextColor: "white"
    property color evenRowColor: "#ffffff"
    property color oddRowColor: "#f8f9fa"
    property color hoverRowColor: "#f0f8f0"
    property color selectedRowColor: "#e8f5e8"
    property color borderColor: "#e0e0e0"
    property color textColor: "#424242"
    property string fontFamily: "Arial"
    property int fontSize: 11
    property int rowHeight: 35
    property int headerHeight: 35

    // Внутренние свойства
    property int selectedRow: -1        // Выбранная строка
    property int hoveredRow: -1         // Выделенная строка
    property bool sortingEnabled: false // Состояние сортировки
    property int sortedColumn: -1       // Какая колонка отсортирована (-1 = нет сортировки)

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        // Заголовок таблицы
        Label {
            text: universalTable.title
            font.bold: true
            font.pixelSize: 14
            Layout.alignment: Qt.AlignLeft
        }

        // Заголовки колонок
        Rectangle {
            Layout.fillWidth: true
            height: universalTable.headerHeight
            color: universalTable.headerColor
            border.color: universalTable.headerBorderColor

            Row {
                anchors.fill: parent

                Repeater {
                    model: universalTable.columnHeaders

                    Rectangle {
                        width: parent.width * modelData.width
                        height: parent.height
                        color: headerMouseArea.containsMouse ? universalTable.headerHoverColor : universalTable.headerColor
                        border.width: 1
                        border.color: universalTable.headerBorderColor

                        MouseArea {
                            id: headerMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                if (universalTable.sortingEnabled) {
                                    universalTable.sortedColumn = index
                                    if (universalTable.onSort) {
                                        universalTable.onSort(index)
                                    }
                                } else {
                                    // Если сортировка запрещена, сбрасываем состояние
                                    universalTable.sortedColumn = -1
                                }
                            }
                        }

                        Row {
                            anchors.centerIn: parent
                            spacing: 5

                            Label {
                                text: modelData.text
                                color: universalTable.headerTextColor
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Label {
                                text: "↕"
                                color: "#90a4ae"
                                font.pixelSize: 12
                                anchors.verticalCenter: parent.verticalCenter
                                visible: universalTable.sortedColumn === index && universalTable.sortingEnabled
                            }
                        }
                    }
                }
            }
        }

        // Таблица данных
        TableView {
            id: tableView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: universalTable.tableModel
            reuseItems: false

            // Глобальная MouseArea для отслеживания hover
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton

                onPositionChanged: {
                    // Вычисляем строку под курсором
                    let rowUnderMouse = Math.floor(mouseY / universalTable.rowHeight)
                    if (rowUnderMouse >= 0 && universalTable.tableModel && rowUnderMouse < universalTable.tableModel.rowCount()) {
                        universalTable.hoveredRow = rowUnderMouse
                    } else {
                        universalTable.hoveredRow = -1
                    }
                }

                onExited: {
                    universalTable.hoveredRow = -1
                }
            }

            columnWidthProvider: function (column) {
                if (column < universalTable.columnHeaders.length) {
                    return width * universalTable.columnHeaders[column].width
                }
                return 100
            }

            rowHeightProvider: function (row) {
                return universalTable.rowHeight
            }

            delegate: Rectangle {
                id: cellDelegate
                implicitWidth: 100
                implicitHeight: universalTable.rowHeight

                color: {
                    if (row === universalTable.selectedRow) {
                        return universalTable.selectedRowColor
                    }
                    if (row === universalTable.hoveredRow) {
                        return universalTable.hoverRowColor
                    }
                    if (row % 2 === 0) {
                        return universalTable.evenRowColor
                    }
                    return universalTable.oddRowColor
                }

                border.color: universalTable.borderColor
                border.width: 1

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        universalTable.selectedRow = row

                        // Вызываем callback для клика по ячейке
                        if (universalTable.onCellClicked) {
                            let cellValue = model.display !== undefined ? model.display : ""
                            universalTable.onCellClicked(row, column, cellValue)
                        }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    anchors.margins: 5
                    text: model.display !== undefined ? model.display : ""

                    // Применяем стилизацию если есть функция стилизации
                    property var style: universalTable.cellStyler ?
                        universalTable.cellStyler(row, column, model.display) :
                        {color: universalTable.textColor, bold: false}

                    color: style.color || universalTable.textColor
                    font.bold: style.bold || false
                    font.family: universalTable.fontFamily
                    font.pixelSize: universalTable.fontSize
                    elide: Text.ElideRight
                    width: parent.width - 10
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
