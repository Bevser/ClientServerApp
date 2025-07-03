import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import enums 1.0

ApplicationWindow {
    id: root
    width: 1280
    height: 800
    minimumHeight: 600
    minimumWidth: 800
    visible: true
    title: "Сервер управления"
    color: "#f0f0f0"

    // Диалог конфигурации
    ConfigurationDialog {
        id: configDialog
        anchors.centerIn: parent

        onConfigurationAccepted: function(message) {
            viewModel.updateClientConfiguration(message)
        }
    }

    // Отладочная информация
    Component.onCompleted: {
        console.log("Main window loaded")
        console.log("viewModel:", viewModel)
        if (viewModel && viewModel.clientTableModel) {
            console.log("clientTableModel:", viewModel.clientTableModel)
            console.log("clientTableModel.columnHeaders:", viewModel.clientTableModel.columnHeaders)
            console.log("clientTableModel.columnWidths:", viewModel.clientTableModel.columnWidths)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Панель управления
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Запустить сервер"
                onClicked: viewModel.startServer(AppEnums.TCP, 12345)
                highlighted: true
            }

            Button {
                text: "Остановить сервер"
                onClicked: viewModel.stopServer(AppEnums.TCP, 12345)
            }

            Rectangle {
                width: 2
                height: parent.height * 0.6
                color: "#ccc"
            }

            Button {
                text: "Старт всем клиентам"
                onClicked: viewModel.startAllClients()
                background: Rectangle {
                    color: "#4CAF50"
                    radius: 3
                    border.color: "#388E3C"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: "Стоп всем клиентам"
                onClicked: viewModel.stopAllClients()
                background: Rectangle {
                    color: "#f44336"
                    radius: 3
                    border.color: "#d32f2f"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "Удалить отключенных"
                onClicked: viewModel.removeDisconnectedClients()
                background: Rectangle {
                    color: "#ff9800"
                    radius: 3
                    border.color: "#f57c00"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            // Отладочная кнопка
            Button {
                text: "Debug"
                visible: false
                onClicked: {
                    console.log("=== DEBUG INFO ===")
                    console.log("viewModel:", viewModel)
                    if (viewModel) {
                        console.log("clientTableModel:", viewModel.clientTableModel)
                        if (viewModel.clientTableModel) {
                            console.log("columnHeaders:", viewModel.clientTableModel.columnHeaders)
                            console.log("columnWidths:", viewModel.clientTableModel.columnWidths)
                            console.log("rowCount:", viewModel.clientTableModel.rowCount())
                        }
                        console.log("dataTableModel:", viewModel.dataTableModel)
                        if (viewModel.dataTableModel) {
                            console.log("data columnHeaders:", viewModel.dataTableModel.columnHeaders)
                            console.log("data columnWidths:", viewModel.dataTableModel.columnWidths)
                            console.log("data rowCount:", viewModel.dataTableModel.rowCount())
                        }
                    }
                }
                background: Rectangle {
                    color: "#9C27B0"
                    radius: 3
                    border.color: "#7B1FA2"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 10
                }
            }
        }

        // Основная область (клиенты + данные)
        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            // Левая панель — клиенты
            Frame {
                SplitView.preferredWidth: 470
                SplitView.minimumWidth: 320

                UniversalTable {
                    id: clientTable
                    anchors.fill: parent
                    title: "Клиенты"
                    tableModel: viewModel ? viewModel.clientTableModel : null

                    // Используем данные из модели
                    columnWidths: viewModel && viewModel.clientTableModel ? viewModel.clientTableModel.columnWidths : []
                    columnHeaders: viewModel && viewModel.clientTableModel ? viewModel.clientTableModel.columnHeaders : []

                    Component.onCompleted: {
                        console.log("Client table completed")
                        console.log("tableModel:", tableModel)
                        console.log("columnHeaders:", columnHeaders)
                        console.log("columnWidths:", columnWidths)
                    }

                    onCellDoubleClicked: function(row, model) {
                        console.log("Cell double clicked, row:", row)
                        let rowData = viewModel.clientTableModel.getRowData(row)
                        console.log("Row data:", JSON.stringify(rowData))
                        if (rowData) {
                            configDialog.openWithData(rowData)
                        }
                    }

                    onHeaderClicked: function(column) {
                        console.log("Header clicked, column:", column)
                        viewModel.sortClients(column)
                    }

                    // Кастомная функция для стилизации ячеек статуса
                    cellStyler: function(row, column, value) {
                        // Колонка статуса (индекс 2)
                        if (column === 2) {
                            if (value === AppEnums.statusToString(AppEnums.CONNECTED)) {
                                return {color: "#4CAF50", bold: true}
                            } else if (value === AppEnums.statusToString(AppEnums.AUTHORIZING)) {
                                return {color: "#FF9800", bold: true}
                            } else if (value === AppEnums.statusToString(AppEnums.DISCONNECTED)) {
                                return {color: "#f44336", bold: true}
                            }
                        }
                        // Колонка отправки (индекс 3)
                        if (column === 3) {
                            if (value === "Да") {
                                return {color: "#4CAF50", bold: true}
                            } else {
                                return {color: "#f44336", bold: true}
                            }
                        }
                        return {color: "#424242", bold: false}
                    }
                }
            }

            // Правая панель — данные и лог
            SplitView {
                SplitView.fillWidth: true
                orientation: Qt.Vertical
                SplitView.preferredWidth: 800
                SplitView.minimumWidth: 400

                // Верхняя часть — таблица данных
                Frame {
                    SplitView.fillHeight: true
                    SplitView.preferredHeight: 600
                    SplitView.minimumHeight: 200

                    UniversalTable {
                        id: dataTable
                        anchors.fill: parent
                        title: "Данные"
                        tableModel: viewModel ? viewModel.dataTableModel : null

                        // Используем данные из модели
                        columnWidths: viewModel && viewModel.dataTableModel ? viewModel.dataTableModel.columnWidths : []
                        columnHeaders: viewModel && viewModel.dataTableModel ? viewModel.dataTableModel.columnHeaders : []

                        Component.onCompleted: {
                            console.log("Data table completed")
                            console.log("tableModel:", tableModel)
                            console.log("columnHeaders:", columnHeaders)
                            console.log("columnWidths:", columnWidths)
                        }

                        onHeaderClicked: function(column) {
                            console.log("Data header clicked, column:", column)
                            viewModel.sortData(column)
                        }

                        // Кастомная функция для стилизации ячеек типа сообщения
                        cellStyler: function(row, column, value) {
                            // Колонка типа (индекс 2)
                            if (column === 2) {                               

                                if (value === "NetworkMetrics") {
                                    return {color: "#0891b2", bold: false}
                                } else if (value === "DeviceStatus") {
                                    return {color: "#2196F3", bold: false}
                                } else if (value === "Log") {
                                    return {color: "#8f4cf6", bold: false}
                                }
                            }
                            return {color: "#424242", bold: false}
                        }
                    }
                }

                // Нижняя часть — лог
                Frame {
                    SplitView.preferredHeight: 200
                    SplitView.minimumHeight: 100

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 5

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "Лог"
                                font.bold: true
                                font.pixelSize: 14
                            }

                            Item { Layout.fillWidth: true }

                            Button {
                                text: "Очистить"
                                onClicked: viewModel.clearLog()
                                background: Rectangle {
                                    color: "#607D8B"
                                    radius: 3
                                    border.color: "#455A64"
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 10
                                }
                            }
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            TextArea {
                                id: logArea
                                text: viewModel ? viewModel.logText : ""
                                readOnly: true
                                wrapMode: TextArea.Wrap
                                font.family: "Consolas, Monaco, monospace"
                                font.pixelSize: 10
                                color: "#333"
                                selectByMouse: true
                                background: Rectangle {
                                    color: "#fafafa"
                                    border.color: "#ddd"
                                }

                                // Автоскролл к началу при обновлении
                                onTextChanged: {
                                    logArea.cursorPosition = 0
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
