import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ServerApp

Dialog {
    id:     serverDialog
    title:  "Управление серверами"
    modal:  true
    width:  650
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
            font.pixelSize: AppTheme.normalFontSize

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Label {
                    text: "Тип:"
                    font.pixelSize: AppTheme.normalFontSize
                }
                ComboBox {
                    id: serverTypeCombo
                    model: [
                        { text: "TCP",          value: AppEnums.TCP },
                        //{ text: "UDP",          value: AppEnums.UDP },
                        //{ text: "MODBUS TCP",   value: AppEnums.MODBUS_TCP },
                        //{ text: "MODBUS RTU",   value: AppEnums.MODBUS_RTU }
                    ]
                    textRole: "text"
                    valueRole: "value"
                    currentIndex: 0
                    implicitWidth: 140
                    font.pixelSize: AppTheme.normalFontSize
                }

                Label {
                    text: "Порт:"
                    font.pixelSize: AppTheme.normalFontSize
                }
                SpinBox {
                    id: portSpinBox
                    editable: true
                    from: 1024
                    to: 65535
                    value: 12345
                    implicitWidth: 150
                    font.pixelSize: AppTheme.normalFontSize
                }

                Button {
                    text: "Добавить"
                    highlighted: true
                    enabled: serverDialog.model ? serverDialog.model.canAddServer(serverTypeCombo.currentValue, portSpinBox.value) : false
                    font.pixelSize: AppTheme.normalFontSize
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
            font.pixelSize: AppTheme.normalFontSize

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
                        color: AppTheme.fieldBackground
                        border.color: AppTheme.fieldBorder
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
                                    font.pixelSize: AppTheme.normalFontSize
                                }

                                Label {
                                    text: "Порт: " + model.port
                                    font.pixelSize: AppTheme.fontSize
                                    color: AppTheme.secondaryText
                                }

                                Label {
                                    text: "Статус: " + model.statusText
                                    font.pixelSize: AppTheme.fontSize
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
                                            if (!parent.enabled) return AppTheme.disabledButtonBackground
                                            if (model.canStop) {
                                                return parent.down ? AppTheme.stopButtonPress :
                                                      (parent.hovered ? AppTheme.stopButtonHover : AppTheme.stopButtonNormal)
                                            } else {
                                                return parent.down ? AppTheme.startButtonPress :
                                                      (parent.hovered ? AppTheme.startButtonHover : AppTheme.startButtonNormal)
                                            }
                                        }
                                        radius: 3
                                        border.color: {
                                            if (!parent.enabled) return AppTheme.disabledButtonBorder
                                            return model.canStop ? AppTheme.stopButtonBorderColor : AppTheme.startButtonBorderColor
                                        }
                                    }

                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.enabled ? "white" : AppTheme.disabledButtonText
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: AppTheme.fontSize
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
                                               (parent.down ? AppTheme.deleteButtonPress :
                                               (parent.hovered ? AppTheme.deleteButtonHover : AppTheme.deleteButtonNormal)) :
                                               AppTheme.disabledButtonBackground
                                        radius: 3
                                        border.color: parent.enabled ? AppTheme.deleteButtonBorderColor : AppTheme.disabledButtonBorder
                                    }

                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.enabled ? "white" : AppTheme.disabledButtonText
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize: AppTheme.fontSize
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
                        color: AppTheme.placeholderText
                        font.pixelSize: AppTheme.normalFontSize
                        visible: serverListView.model ? serverListView.model.count === 0 : true
                    }
                }
            }
        }
    }
}
