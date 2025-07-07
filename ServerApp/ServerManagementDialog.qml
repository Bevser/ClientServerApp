import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import enums 1.0

Dialog {
    id:     serverDialog
    title:  "Управление серверами"
    modal:  true
    width:  650
    height: 500
    anchors.centerIn: parent
    standardButtons: Dialog.Close

    // Темы и стили
    readonly property QtObject theme: QtObject {
        readonly property color toolButtonPressed:          "#d4d4d4"
        readonly property color toolButtonHovered:          "#e8e8e8"
        readonly property color toolButtonBorder:           "#c0c0c0"
        readonly property color toolButtonBorderPressed:    "#a0a0a0"
        readonly property color separatorColor:             "#d0d0d0"

        readonly property color startButtonPressed:         "#c8e6c9"
        readonly property color startButtonHovered:         "#e8f5e8"
        readonly property color startButtonBorder:          "#81c784"
        readonly property color startButtonBorderPressed:   "#4caf50"

        readonly property color stopButtonPressed:          "#ffcdd2"
        readonly property color stopButtonHovered:          "#ffebee"
        readonly property color stopButtonBorder:           "#e57373"
        readonly property color stopButtonBorderPressed:    "#f44336"

        readonly property color clearButtonNormal:          "#607D8B"
        readonly property color clearButtonHovered:         "#78909C"
        readonly property color clearButtonPressed:         "#546E7A"
        readonly property color clearButtonBorder:          "#455A64"

        readonly property color logBackground:              "#fafafa"
        readonly property color logBorder:                  "#ddd"
        readonly property color logText:                    "#333"

        // Дополнительные цвета для диалога
        readonly property color headerBackground:           "#f8f9fa"
        readonly property color headerBorder:               "#e0e0e0"
        readonly property color secondaryText:              "#666"
        readonly property color connectedStatus:            "#4CAF50"
        readonly property color disconnectedStatus:         "#f44336"
        readonly property color fieldBackground:            "#ffffff"
        readonly property color fieldBorder:                "#e0e0e0"
        readonly property color readOnlyBackground:         "#fff3e0"
        readonly property color placeholderText:            "#999"

        // Цвета для кнопок сервера
        readonly property color startButtonNormal:          "#66BB6A"
        readonly property color startButtonHover:           "#4CAF50"
        readonly property color startButtonPress:           "#388E3C"
        readonly property color startButtonBorderColor:     "#388E3C"

        readonly property color stopButtonNormal:           "#ff5722"
        readonly property color stopButtonHover:            "#f44336"
        readonly property color stopButtonPress:            "#d32f2f"
        readonly property color stopButtonBorderColor:      "#d32f2f"

        readonly property color deleteButtonNormal:         "#A1887F"
        readonly property color deleteButtonHover:          "#8D6E63"
        readonly property color deleteButtonPress:          "#795548"
        readonly property color deleteButtonBorderColor:    "#5D4037"

        readonly property color disabledButtonBackground:   "#e0e0e0"
        readonly property color disabledButtonBorder:       "#bdbdbd"
        readonly property color disabledButtonText:         "#9e9e9e"

        readonly property string monoFont:                  "Consolas, Monaco, monospace"
        readonly property int toolButtonSize:               24
        readonly property int smallFontSize:                10
        readonly property int normalFontSize:               14
    }

    property alias model: serverListView.model

    contentItem: ColumnLayout {
        spacing: 15

        // Секция добавления нового сервера
        GroupBox {
            title: "Добавить новый сервер"
            Layout.fillWidth: true
            font.pixelSize: serverDialog.theme.normalFontSize

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Label {
                    text: "Тип:"
                    font.pixelSize: serverDialog.theme.normalFontSize
                }
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
                    font.pixelSize: serverDialog.theme.normalFontSize
                }

                Label {
                    text: "Порт:"
                    font.pixelSize: serverDialog.theme.normalFontSize
                }
                SpinBox {
                    id: portSpinBox
                    editable: true
                    from: 1024
                    to: 65535
                    value: 12345
                    implicitWidth: 150
                    font.pixelSize: serverDialog.theme.normalFontSize
                }

                Button {
                    text: "Добавить"
                    highlighted: true
                    enabled: serverDialog.model ? serverDialog.model.canAddServer(serverTypeCombo.currentValue, portSpinBox.value) : false
                    font.pixelSize: serverDialog.theme.normalFontSize
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
            font.pixelSize: serverDialog.theme.normalFontSize

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
                        color: serverDialog.theme.fieldBackground
                        border.color: serverDialog.theme.fieldBorder
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
                                    font.pixelSize: serverDialog.theme.normalFontSize
                                }

                                Label {
                                    text: "Порт: " + model.port
                                    font.pixelSize: serverDialog.theme.smallFontSize + 2
                                    color: serverDialog.theme.secondaryText
                                }

                                Label {
                                    text: "Статус: " + model.statusText
                                    font.pixelSize: serverDialog.theme.smallFontSize + 2
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
                                            if (!parent.enabled) return serverDialog.theme.disabledButtonBackground
                                            if (model.canStop) {
                                                return parent.down ? serverDialog.theme.stopButtonPress :
                                                      (parent.hovered ? serverDialog.theme.stopButtonHover : serverDialog.theme.stopButtonNormal)
                                            } else {
                                                return parent.down ? serverDialog.theme.startButtonPress :
                                                      (parent.hovered ? serverDialog.theme.startButtonHover : serverDialog.theme.startButtonNormal)
                                            }
                                        }
                                        radius: 3
                                        border.color: {
                                            if (!parent.enabled) return serverDialog.theme.disabledButtonBorder
                                            return model.canStop ? serverDialog.theme.stopButtonBorderColor : serverDialog.theme.startButtonBorderColor
                                        }
                                    }

                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.enabled ? "white" : serverDialog.theme.disabledButtonText
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: serverDialog.theme.smallFontSize + 1
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
                                               (parent.down ? serverDialog.theme.deleteButtonPress :
                                               (parent.hovered ? serverDialog.theme.deleteButtonHover : serverDialog.theme.deleteButtonNormal)) :
                                               serverDialog.theme.disabledButtonBackground
                                        radius: 3
                                        border.color: parent.enabled ? serverDialog.theme.deleteButtonBorderColor : serverDialog.theme.disabledButtonBorder
                                    }

                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.enabled ? "white" : serverDialog.theme.disabledButtonText
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: serverDialog.theme.smallFontSize + 1
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
                        color: serverDialog.theme.placeholderText
                        font.pixelSize: serverDialog.theme.normalFontSize
                        visible: serverListView.model ? serverListView.model.count === 0 : true
                    }
                }
            }
        }
    }
}
