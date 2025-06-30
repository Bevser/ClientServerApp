import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: universalTable

    property string title: "Таблица"
    property var tableModel: null
    property var columnWidths: []  // Массив относительных ширин колонок
    property var columnHeaders: [] // Массив заголовков колонок
    property var onCellClicked: null
    property var onCellDoubleClicked: null
    property var onHeaderClicked: null
    property var cellStyler: null

    // Настройки стилей
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

    // Состояние таблицы
    property int selectedRow: -1
    property int hoveredRow: -1
    property int sortedColumn: -1

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Заголовок таблицы
        Label {
            text: universalTable.title
            font.bold: true
            font.pixelSize: 14
            Layout.alignment: Qt.AlignLeft
            Layout.bottomMargin: 5
        }

        // Заголовок колонок
        Rectangle {
            id: headerRow
            Layout.fillWidth: true
            height: universalTable.headerHeight
            color: universalTable.headerColor
            border.color: universalTable.headerBorderColor
            border.width: 1

            Row {
                id: headerRowContent
                anchors.fill: parent
                anchors.margins: 0

                Repeater {
                    id: headerRepeater
                    model: universalTable.columnHeaders && universalTable.columnHeaders.length > 0 ? universalTable.columnHeaders : []

                    Rectangle {
                        id: headerCell
                        width: {
                            if (!universalTable.columnWidths || universalTable.columnWidths.length === 0) {
                                return headerRow.width / Math.max(1, universalTable.columnHeaders.length)
                            }
                            if (index < universalTable.columnWidths.length) {
                                return headerRow.width * universalTable.columnWidths[index]
                            }
                            return headerRow.width / Math.max(1, universalTable.columnHeaders.length)
                        }
                        height: universalTable.headerHeight
                        color: headerMouseArea.containsMouse ? universalTable.headerHoverColor : universalTable.headerColor
                        border.width: index > 0 ? 1 : 0
                        border.color: universalTable.headerBorderColor

                        MouseArea {
                            id: headerMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                universalTable.sortedColumn = index
                                if (universalTable.onHeaderClicked) {
                                    universalTable.onHeaderClicked(index)
                                }
                                console.log("Header clicked:", index, modelData)
                            }
                        }

                        Row {
                            anchors.centerIn: parent
                            spacing: 5

                            Label {
                                text: modelData || ""
                                color: universalTable.headerTextColor
                                font.bold: true
                                font.family: universalTable.fontFamily
                                font.pixelSize: universalTable.fontSize + 1
                                anchors.verticalCenter: parent.verticalCenter
                                elide: Text.ElideRight
                            }

                            Label {
                                text: "↕"
                                color: "#90a4ae"
                                font.pixelSize: 12
                                anchors.verticalCenter: parent.verticalCenter
                                visible: universalTable.sortedColumn === index
                            }
                        }
                    }
                }
            }
        }

        // Контейнер для таблицы данных
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            border.color: universalTable.borderColor
            border.width: 1

            // Таблица данных
            TableView {
                id: tableView
                anchors.fill: parent
                anchors.margins: 0
                clip: true
                model: universalTable.tableModel
                reuseItems: false
                boundsBehavior: Flickable.StopAtBounds

                // Убираем полосы прокрутки по умолчанию
                ScrollBar.horizontal: ScrollBar {
                    id: hbar
                    visible: tableView.contentWidth > tableView.width
                }
                ScrollBar.vertical: ScrollBar {
                    id: vbar
                    visible: tableView.contentHeight > tableView.height
                }

                columnWidthProvider: function (column) {
                    if (!universalTable.columnWidths || universalTable.columnWidths.length === 0) {
                        return width / Math.max(1, universalTable.columnHeaders.length)
                    }
                    if (column < universalTable.columnWidths.length) {
                        return width * universalTable.columnWidths[column]
                    }
                    return width / Math.max(1, universalTable.columnHeaders.length)
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
                        return row % 2 === 0 ? universalTable.evenRowColor : universalTable.oddRowColor
                    }

                    border.color: universalTable.borderColor
                    border.width: 0.5

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true

                        // Используем Timer для задержки сброса hover
                        Timer {
                            id: hoverTimer
                            interval: 50
                            onTriggered: {
                                if (universalTable.hoveredRow === row) {
                                    universalTable.hoveredRow = -1
                                }
                            }
                        }

                        onEntered: {
                            hoverTimer.stop()
                            universalTable.hoveredRow = row
                        }

                        onExited: {
                            // Запускаем таймер вместо немедленного сброса
                            hoverTimer.start()
                        }

                        onClicked: {
                            universalTable.selectedRow = row
                            if (universalTable.onCellClicked) {
                                universalTable.onCellClicked(row, column, model)
                            }
                        }

                        onDoubleClicked: {
                            if (universalTable.onCellDoubleClicked) {
                                universalTable.onCellDoubleClicked(row, model)
                            }
                        }
                    }
                    Text {
                        anchors.centerIn: parent
                        anchors.margins: 5
                        text: {
                            if (model && model.display !== undefined) {
                                return model.display.toString()
                            }
                            return ""
                        }

                        property var style: universalTable.cellStyler ?
                            universalTable.cellStyler(row, column, model && model.display ? model.display.toString() : "") :
                            {color: universalTable.textColor, bold: false}

                        color: style.color || universalTable.textColor
                        font.bold: style.bold || false
                        font.family: universalTable.fontFamily
                        font.pixelSize: universalTable.fontSize
                        elide: Text.ElideRight
                        width: parent.width - 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.NoWrap
                    }
                }
            }
        }
    }

    // Отладочная информация
    Component.onCompleted: {
        console.log("UniversalTable initialized:")
        console.log("- columnHeaders:", JSON.stringify(columnHeaders))
        console.log("- columnWidths:", JSON.stringify(columnWidths))
        console.log("- tableModel:", tableModel)
    }

    onColumnHeadersChanged: {
        console.log("Column headers changed:", JSON.stringify(columnHeaders))
    }

    onColumnWidthsChanged: {
        console.log("Column widths changed:", JSON.stringify(columnWidths))
    }

    onTableModelChanged: {
        console.log("Table model changed:", tableModel)
    }
}
