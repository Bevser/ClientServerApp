import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import enums 1.0

Dialog {
    id: serverDialog
    title: "Управление серверами"
    modal: true
    width: 650
    height: 500
    anchors.centerIn: parent
    standardButtons: Dialog.Close

    property alias model: serverListView.model

    contentItem: ColumnLayout {
        spacing: 15

        // Секция добавления нового сервера
        GroupBox {
            title: "Добавить новый сервер"
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Label { text: "Тип:" }
                ComboBox {
                    id: serverTypeCombo
                    model: [
                        { text: "TCP",          value: AppEnums.TCP },
                        { text: "UDP",          value: AppEnums.UDP },
                        { text: "MODBUS TCP",   value: AppEnums.MODBUS_TCP },
                        { text: "MODBUS RTU",   value: AppEnums.MODBUS_RTU }
                    ]
                    textRole: "text"
                    valueRole: "value"
                    currentIndex: 0
                    implicitWidth: 140
                }

                Label { text: "Порт:" }
                SpinBox {
                    id: portSpinBox
                    editable: true
                    from: 1024
                    to: 65535
                    value: 12345
                    implicitWidth: 150
                }

                Button {
                    text: "Добавить"
                    highlighted: true
                    enabled: serverDialog.model ? serverDialog.model.canAddServer(serverTypeCombo.currentValue, portSpinBox.value) : false
                    onClicked: {
                        viewModel.addServerToList(serverTypeCombo.currentValue, portSpinBox.value)
                        portSpinBox.value++
                    }
                }
            }
        }

        // Список серверов
        GroupBox {
            title: "Конфигурация серверов"
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollView {
                anchors.fill: parent
                clip: true

                ListView {
                    id: serverListView
                    model: serverDialog.model
                    spacing: 5

                    delegate: Rectangle {
                        id: serverItem
                        width: ListView.view.width
                        height: 80
                        color: "#ffffff"
                        border.color: "#e0e0e0"
                        border.width: 1
                        radius: 5

                        // Цветовая индикация статуса - теперь берем цвет из модели
                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: 4
                            color: model.statusColor
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 15
                            spacing: 15

                            // Информация о сервере
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5

                                Label {
                                    text: model.serverInfo
                                    font.bold: true
                                    font.pixelSize: 14
                                }

                                Label {
                                    text: "Порт: " + model.port
                                    font.pixelSize: 12
                                    color: "#666"
                                }

                                Label {
                                    text: "Статус: " + model.statusText
                                    font.pixelSize: 12
                                    color: model.statusColor
                                }
                            }

                            // Кнопки управления
                            RowLayout {
                                spacing: 10
                                Layout.alignment: Qt.AlignRight

                                Button {
                                    text: model.canStop ? "Остановить" : "Запустить"
                                    enabled: model.canStart || model.canStop
                                    highlighted: model.canStart
                                    implicitWidth: 90

                                    background: Rectangle {
                                        color: {
                                            if (!parent.enabled) return "#e0e0e0"
                                            if (model.canStop) {
                                                return parent.down ? "#d32f2f" : (parent.hovered ? "#f44336" : "#ff5722")
                                            } else {
                                                return parent.down ? "#388E3C" : (parent.hovered ? "#4CAF50" : "#66BB6A")
                                            }
                                        }
                                        radius: 3
                                        border.color: {
                                            if (!parent.enabled) return "#bdbdbd"
                                            return model.canStop ? "#d32f2f" : "#388E3C"
                                        }
                                    }

                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.enabled ? "white" : "#9e9e9e"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: 11
                                    }

                                    onClicked: {
                                        if (model.canStop) {
                                            viewModel.stopServer(model.type, model.port)
                                        } else if (model.canStart) {
                                            viewModel.startServer(model.type, model.port)
                                        }
                                    }
                                }

                                Button {
                                    text: "Удалить"
                                    enabled: model.canDelete
                                    implicitWidth: 70

                                    background: Rectangle {
                                        color: parent.enabled ?
                                               (parent.down ? "#795548" : (parent.hovered ? "#8D6E63" : "#A1887F")) :
                                               "#e0e0e0"
                                        radius: 3
                                        border.color: parent.enabled ? "#5D4037" : "#bdbdbd"
                                    }

                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.enabled ? "white" : "#9e9e9e"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: 11
                                    }

                                    onClicked: {
                                        viewModel.removeServerFromList(model.type, model.port)
                                    }
                                }
                            }
                        }
                    }

                    // Сообщение когда нет серверов
                    Label {
                        anchors.centerIn: parent
                        text: "Нет добавленных серверов"
                        color: "#999"
                        font.pixelSize: 14
                        visible: serverListView.model ? serverListView.model.count === 0 : true
                    }
                }
            }
        }
    }
}
