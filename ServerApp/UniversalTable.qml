import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    // Публичные свойства
    property string title:      "Таблица"
    property var tableModel:    null
    property var columnWidths:  []
    property var columnHeaders: []

    // Колбэки
    property var onCellClicked: null
    property var onCellDoubleClicked: null
    property var onHeaderClicked: null

    // Настройки стилей (сгруппированы)
    readonly property QtObject theme: QtObject {
        readonly property color headerColor:        "#37474f"
        readonly property color headerHoverColor:   "#455a64"
        readonly property color headerBorderColor:  "#263238"
        readonly property color headerTextColor:    "white"

        readonly property color evenRowColor:       "#ffffff"
        readonly property color oddRowColor:        "#f8f9fa"
        readonly property color hoverRowColor:      "#f0f8f0"
        readonly property color selectedRowColor:   "#dcf0dc"

        readonly property color borderColor:        "#e0e0e0"
        readonly property color textColor:          "#424242"

        readonly property string fontFamily:        "Arial"
        readonly property int fontSize:             11
        readonly property int rowHeight:            35
        readonly property int headerHeight:         35
    }

    // Состояние таблицы
    property int selectedRow: -1
    property int hoveredRow: -1
    property int sortedColumn: -1

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
            font.pixelSize: 14
            Layout.alignment: Qt.AlignLeft
            Layout.bottomMargin: 5
            visible: root.title !== ""
        }

        // Заголовок колонок
        Rectangle {
            id: headerContainer
            Layout.fillWidth: true
            height: root.theme.headerHeight
            color: root.theme.headerColor
            border.color: root.theme.headerBorderColor
            border.width: 1
            visible: root.hasHeaders

            Row {
                anchors.fill: parent

                Repeater {
                    model: root.columnHeaders

                    Rectangle {
                        id: headerCell
                        width: root.getColumnWidth(index, headerContainer.width)
                        height: root.theme.headerHeight
                        color: headerMouseArea.containsMouse ? root.theme.headerHoverColor : root.theme.headerColor
                        border.width: index > 0 ? 1 : 0
                        border.color: root.theme.headerBorderColor

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
                                color: root.theme.headerTextColor
                                font.bold: true
                                font.family: root.theme.fontFamily
                                font.pixelSize: root.theme.fontSize + 1
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
            border.color: root.theme.borderColor
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
                    return root.theme.rowHeight
                }

                delegate: Rectangle {
                    id: cellDelegate
                    implicitWidth: 100
                    implicitHeight: root.theme.rowHeight

                    // Вычисление цвета
                    readonly property color cellColor: {
                        if (row === root.selectedRow) return root.theme.selectedRowColor
                        if (row === root.hoveredRow) return root.theme.hoverRowColor
                        return row % 2 === 0 ? root.theme.evenRowColor : root.theme.oddRowColor
                    }

                    color: cellColor
                    border.color: root.theme.borderColor
                    border.width: 0.5

                    MouseArea {
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

                    Text {
                        anchors.centerIn: parent
                        anchors.margins: 5
                        text: model.display ? model.display.toString() : ""
                        color: model.statusColor || model.typeColor || root.theme.textColor
                        font.bold: model.isBold || false
                        font.family: root.theme.fontFamily
                        font.pixelSize: root.theme.fontSize
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
