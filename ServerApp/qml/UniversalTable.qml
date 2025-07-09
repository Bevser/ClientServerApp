import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ServerApp

Item {
    id: root

    // Публичные свойства
    property string title:      "Таблица"
    property var tableModel:    null
    property var columnWidths:  []
    property var columnHeaders: []
    property int tooltipColumn: -1

    // Колбэки
    property var onCellClicked:         null
    property var onCellDoubleClicked:   null
    property var onHeaderClicked:       null

    // Состояние таблицы
    property int selectedRow:   -1
    property int hoveredRow:    -1
    property int sortedColumn:  -1

    // Внутренние свойства
    readonly property bool hasHeaders: columnHeaders && columnHeaders.length > 0
    readonly property int columnsCount: hasHeaders ? columnHeaders.length : 0
    readonly property bool hasCustomWidths: columnWidths && columnWidths.length > 0

    // Функции для вычисления ширины колонок
    function getColumnWidth(index, totalWidth) {
        if (!hasCustomWidths || index >= columnWidths.length) {
            return totalWidth / Math.max(1, columnsCount)
        }
        return totalWidth * columnWidths[index]
    }

    // Соединения с моделью
    Connections {
        target: tableModel
        function onResetSorting() {
            root.sortedColumn = -1
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Заголовок таблицы
        Label {
            text: root.title
            font.bold: true
            font.pixelSize: AppTheme.normalFontSize
            Layout.alignment: Qt.AlignLeft
            Layout.bottomMargin: 5
            visible: root.title !== ""
        }

        // Заголовок колонок
        Rectangle {
            id: headerContainer
            Layout.fillWidth: true
            height: AppTheme.headerHeight
            color: AppTheme.headerColor
            border.color: AppTheme.headerBorderColor
            border.width: 1
            visible: root.hasHeaders

            Row {
                anchors.fill: parent

                Repeater {
                    model: root.columnHeaders

                    Rectangle {
                        id: headerCell
                        width: root.getColumnWidth(index, headerContainer.width)
                        height: AppTheme.headerHeight
                        color: headerMouseArea.containsMouse ? AppTheme.headerHoverColor : AppTheme.headerColor
                        border.width: index > 0 ? 1 : 0
                        border.color: AppTheme.headerBorderColor

                        MouseArea {
                            id: headerMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                root.sortedColumn = index
                                if (root.onHeaderClicked) {
                                    root.onHeaderClicked(index)
                                }
                            }
                        }

                        Row {
                            anchors.centerIn: parent
                            spacing: 5

                            Label {
                                text: modelData || ""
                                color: AppTheme.headerTextColor
                                font.bold: true
                                font.family: AppTheme.fontFamily
                                font.pixelSize: AppTheme.fontSize + 1
                                anchors.verticalCenter: parent.verticalCenter
                                elide: Text.ElideRight
                            }

                            Label {
                                text: "↕"
                                color: "#90a4ae"
                                font.pixelSize: 12
                                anchors.verticalCenter: parent.verticalCenter
                                visible: root.sortedColumn === index
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
            border.color: AppTheme.borderColor
            border.width: 1

            TableView {
                id: tableView
                anchors.fill: parent
                clip: true
                model: root.tableModel
                boundsBehavior: Flickable.StopAtBounds

                // Полосы прокрутки
                ScrollBar.horizontal: ScrollBar {
                    policy: tableView.contentWidth > tableView.width ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                }
                ScrollBar.vertical: ScrollBar {
                    policy: tableView.contentHeight > tableView.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                }

                columnWidthProvider: function (column) {
                    return root.getColumnWidth(column, width)
                }

                rowHeightProvider: function (row) {
                    return AppTheme.rowHeight
                }

                delegate: Rectangle {
                    id: cellDelegate
                    implicitWidth: 100
                    implicitHeight: AppTheme.rowHeight

                    // Вычисление цвета
                    readonly property color cellColor: {
                        if (row === root.selectedRow) return AppTheme.selectedRowColor
                        if (row === root.hoveredRow) return AppTheme.hoverRowColor
                        return row % 2 === 0 ? AppTheme.evenRowColor : AppTheme.oddRowColor
                    }

                    color: cellColor
                    border.color: AppTheme.borderColor
                    border.width: 0.5

                    MouseArea {
                        id: cellMouseArea
                        anchors.fill: parent
                        hoverEnabled: true

                        // Timer для корректного управления hover при быстром перемещении мыши
                        Timer {
                            id: hoverResetTimer
                            interval: 50
                            onTriggered: {
                                if (root.hoveredRow === row) {
                                    root.hoveredRow = -1
                                }
                            }
                        }

                        onEntered: {
                            hoverResetTimer.stop()
                            root.hoveredRow = row
                        }

                        onExited: {
                            // Используем Timer для предотвращения мерцания при переходе между ячейками
                            hoverResetTimer.start()
                        }

                        onClicked: {
                            root.selectedRow = row
                            if (root.onCellClicked) {
                                root.onCellClicked(row, column, model)
                            }
                        }

                        onDoubleClicked: {
                            if (root.onCellDoubleClicked) {
                                root.onCellDoubleClicked(row, model)
                            }
                        }
                    }

                    ToolTip {
                        id: cellToolTip
                        visible: cellMouseArea.containsMouse &&
                                root.tooltipColumn >= 0 &&
                                column === root.tooltipColumn &&
                                model.display &&
                                model.display.toString().length > 0
                        delay: 500  // Задержка перед показом подсказки (мс)

                        text: model.display ? model.display.toString() : ""

                        // Стилизация подсказки
                        background: Rectangle {
                            color: AppTheme.tooltipBackgroundColor
                            border.color: AppTheme.tooltipBorderColor
                            border.width: 1
                            radius: 4
                        }

                        contentItem: Text {
                            text: cellToolTip.text
                            color: AppTheme.tooltipTextColor
                            font.family: AppTheme.fontFamily
                            font.pixelSize: AppTheme.fontSize
                            wrapMode: Text.Wrap
                            maximumLineCount: 10
                        }

                        // Позиционирование подсказки
                        x: cellMouseArea.mouseX + 10
                        y: cellMouseArea.mouseY + 10
                    }

                    Text {
                        anchors.centerIn: parent
                        anchors.margins: 5
                        text: model.display ? model.display.toString() : ""
                        color: model.statusColor || model.typeColor || AppTheme.textColor
                        font.bold: model.isBold || false
                        font.family: AppTheme.fontFamily
                        font.pixelSize: AppTheme.fontSize
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
}
