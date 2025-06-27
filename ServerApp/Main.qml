import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1280
    height: 800
    visible: true
    title: "Сервер управления"
    color: "#f0f0f0"
    property bool sortingEnabled: false

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
                onClicked: viewModel.startServer()
                highlighted: true
            }
            Button {
                text: "Остановить сервер"
                onClicked: viewModel.stopServer()
            }
            Item { Layout.fillWidth: true }
            Button {
                text: "Старт всем клиентам"
                onClicked:
                {
                    root.sortingEnabled = false
                    viewModel.startAllClients()
                }
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
                onClicked:
                {
                    root.sortingEnabled = true
                    viewModel.stopAllClients()
                }
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
        }

        // Основная область (клиенты + данные)
        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            // Левая панель — клиенты
            Frame {
                SplitView.preferredWidth: 450

                UniversalTable {
                    anchors.fill: parent
                    title: "Подключенные клиенты"

                    selectedRowColor: "#e8f5e8"
                    hoverRowColor: "#f0f8f0"
                    sortingEnabled: root.sortingEnabled


                    columnHeaders: [
                        {text: "ID Клиента", width: 0.33},
                        {text: "IP Адрес", width: 0.33},
                        {text: "Статус", width: 0.34}
                    ]

                    tableModel: viewModel.clientTableModel

                    onSort: function(columnIndex) {
                        viewModel.sortClients(columnIndex)
                    }

                    onCellClicked: function(row, column, value) {
                        console.log("Клик по клиенту:", row, value)
                    }

                    // Кастомная стилизация для колонки статуса
                    cellStyler: function(row, column, value) {
                        if (column === 2) { // Колонка статуса
                            return {
                                color: value === "Подключен" ? "#2e7d32" : "#c62828",
                                bold: value === "Подключен"
                            }
                        }
                        return {color: "#424242", bold: false}
                    }
                }
            }

            // Правая панель — данные
            Frame {
                SplitView.fillWidth: true

                UniversalTable {
                    anchors.fill: parent
                    title: "Полученные данные"

                    selectedRowColor: "#fff3e0"
                    hoverRowColor: "#fffbf0"
                    sortingEnabled: root.sortingEnabled

                    columnHeaders: [
                        {text: "Время", width: 0.2},
                        {text: "ID Клиента", width: 0.2},
                        {text: "Тип", width: 0.2},
                        {text: "Содержимое", width: 0.4}
                    ]

                    tableModel: viewModel.dataTableModel
                    fontFamily: "monospace"
                    fontSize: 11

                    onSort: function(columnIndex) {
                        viewModel.sortData(columnIndex)
                    }

                    onCellClicked: function(row, column, value) {
                        console.log("Данные:", row, column, value)
                    }

                    // Простая стилизация для таблицы данных
                    cellStyler: function(row, column, value) {
                        return {color: "#424242", bold: false}
                    }
                }
            }
        }

        // Лог событий
        Frame {
            Layout.fillWidth: true
            Layout.minimumHeight: 150
            Layout.preferredHeight: root.height * 0.25
            background: Rectangle {
                color: "#212121"
                border.color: "gray"
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 5

                Label {
                    text: "Лог событий"
                    font.bold: true
                    Layout.alignment: Qt.AlignLeft
                    leftPadding: 5
                    color: "#E0E0E0"
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    TextArea {
                        readOnly: true
                        text: viewModel.logText
                        color: "#E0E0E0"
                        font.family: "monospace"
                        background: null
                    }
                }
            }
        }
    }
}
