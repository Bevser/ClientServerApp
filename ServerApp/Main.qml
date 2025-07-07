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

    ServerManagementDialog {
        id: serverDialog
        anchors.centerIn: parent
        // Ключевая привязка модели
        model: viewModel.serverListModel
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
                text: "Управление серверами"
                onClicked: serverDialog.open()
                highlighted: true
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

                Button {
                    text: "Убрать отключенных"
                    anchors.right: parent.right
                    highlighted: true
                    onClicked: viewModel.removeDisconnectedClients()
                    background: Rectangle {
                        color: parent.down ? "#546E7A" : (parent.hovered ? "#78909C" : "#607D8B")
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

                UniversalTable {
                    id: clientTable
                    anchors.fill: parent
                    title: "Клиенты"
                    tableModel: viewModel ? viewModel.clientTableModel : null

                    // Используем данные из модели
                    columnWidths: viewModel && viewModel.clientTableModel ? viewModel.clientTableModel.columnWidths : []
                    columnHeaders: viewModel && viewModel.clientTableModel ? viewModel.clientTableModel.columnHeaders : []

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

                    Button {
                        text: "Очистить"
                        anchors.right: parent.right
                        highlighted: true
                        onClicked: viewModel.clearData()
                        background: Rectangle {
                            color: parent.down ? "#546E7A" : (parent.hovered ? "#78909C" : "#607D8B")
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
                    }
                }

                // Нижняя часть — лог
                Frame {
                    SplitView.preferredHeight: 200
                    SplitView.minimumHeight: 100

                    Button {
                        text: "Очистить"
                        anchors.right: parent.right
                        highlighted: true
                        onClicked: viewModel.clearLog()
                        background: Rectangle {
                            color: parent.down ? "#546E7A" : (parent.hovered ? "#78909C" : "#607D8B")
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

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 5

                        Label {
                            text: "Лог"
                            font.bold: true
                            font.pixelSize: 14
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
