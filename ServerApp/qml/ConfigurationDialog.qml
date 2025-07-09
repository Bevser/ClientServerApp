import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ServerApp

Dialog {
    id:     configDialog
    title:  "Конфигурация клиента"
    modal:  true
    width:  450
    height: 550

    property var clientData: null
    property bool isClientConnected: false
    property bool allowSending: false
    property bool originalAllowSending: false

    signal configurationAccepted(var message)
    signal sendingToggled(var data)

    function openWithData(data) {
        console.log("Открытие диалога для клиента:", JSON.stringify(data))

        clientData = data

        // Безопасная проверка статуса
        isClientConnected = (data && data.status === AppEnums.CONNECTED)

        allowSending = false
        if (data) {
            if (data.allowSending !== undefined) {
                allowSending = data.allowSending
            } else if (data.allow_sending !== undefined) {
                allowSending = data.allow_sending
            }
        }

        // Сохраняем исходное значение
        originalAllowSending = allowSending

        // Очищаем и заполняем модель конфигурации
        configListModel.clear()

        if (data && data.Configuration) {
            var config = data.Configuration

            // Если конфигурация строка, парсим
            if (typeof config === "string") {
                try {
                    config = JSON.parse(config)
                } catch (e) {
                    console.warn("Ошибка парсинга конфигурации:", e)
                    config = {}
                }
            }

            // Добавляем параметры в модель
            for (var key in config) {
                configListModel.append({
                    "key": key,
                    "value": config[key] ? config[key].toString() : "",
                    "originalValue": config[key] ? config[key].toString() : "",
                    "isReadOnly": false
                })
            }
        }

        // Если нет конфигурации, показываем сообщение
        if (configListModel.count === 0) {
            var message = isClientConnected ?
                "Конфигурация не получена" :
                "Клиент не зарегистрирован"

            configListModel.append({
                "key": "Информация",
                "value": message,
                "originalValue": "",
                "isReadOnly": true
            })
        }

        open()
    }

    // Модель для параметров конфигурации
    ListModel {
        id: configListModel
    }

    header: Rectangle {
        height: 80
        color: AppTheme.headerBackground
        border.color: AppTheme.headerBorder

        RowLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 15

            // Информация о клиенте
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    text: "ID: " + (clientData ? clientData.id : "N/A")
                    font.bold: true
                    font.pixelSize: AppTheme.normalFontSize
                }

                Label {
                    text: "Адрес: " + (clientData ? clientData.address : "N/A")
                    font.pixelSize: AppTheme.smallFontSize
                    color: AppTheme.secondaryText
                }

                Label {
                    text: "Статус: " + (clientData ? AppEnums.clientStatusToString(clientData.status) : "N/A")
                    font.pixelSize: AppTheme.smallFontSize
                    color: isClientConnected ? AppTheme.connectedStatus : AppTheme.disconnectedStatus
                    font.bold: true
                }
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 15

        // Настройка отправки данных
        GroupBox {
            title: "Отправка данных"
            Layout.fillWidth: true
            font.pixelSize: AppTheme.normalFontSize

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Label {
                    text: allowSending ? "Разрешена" : "Запрещена"
                    color: allowSending ? AppTheme.connectedStatus : AppTheme.disconnectedStatus
                    font.bold: true
                    font.pixelSize: AppTheme.normalFontSize
                }

                Item { Layout.fillWidth: true }

                Switch {
                    id: sendingSwitch
                    checked: allowSending
                    enabled: isClientConnected

                    onToggled: {
                        allowSending = checked
                    }
                }
            }
        }

        // Параметры конфигурации
        GroupBox {
            title: "Параметры конфигурации"
            Layout.fillWidth: true
            Layout.fillHeight: true
            font.pixelSize: AppTheme.normalFontSize

            ScrollView {
                anchors.fill: parent
                clip: true

                ListView {
                    model: configListModel
                    spacing: 8

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 50
                        color: model.isReadOnly ? AppTheme.readOnlyBackground : AppTheme.fieldBackground
                        border.color: AppTheme.fieldBorder
                        radius: 4

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10

                            // Название параметра
                            Label {
                                text: model.key || ""
                                font.bold: true
                                font.pixelSize: AppTheme.normalFontSize
                                Layout.preferredWidth: 120
                                Layout.alignment: Qt.AlignTop
                            }

                            // Поле значения
                            TextField {
                                id: valueField
                                validator: DoubleValidator {
                                    bottom: 0
                                    top: 1000
                                    decimals: 3
                                    notation: DoubleValidator.StandardNotation
                                }
                                Layout.fillWidth: true
                                text: model.value || ""
                                placeholderText: "0"
                                enabled: isClientConnected && !model.isReadOnly
                                readOnly: !isClientConnected || model.isReadOnly
                                inputMethodHints: Qt.ImhDigitsOnly
                                font.pixelSize: AppTheme.normalFontSize

                                onTextChanged: {
                                    if (!model.isReadOnly && isClientConnected) {
                                        configListModel.setProperty(index, "value", text)
                                    }
                                }
                            }

                            // Кнопка сброса
                            Button {
                                text: "↺"
                                implicitWidth: 30
                                implicitHeight: 30
                                enabled: isClientConnected && !model.isReadOnly
                                visible: !model.isReadOnly
                                ToolTip.text: "Сбросить"
                                ToolTip.visible: hovered
                                font.pixelSize: AppTheme.normalFontSize

                                onClicked: {
                                    if (model.originalValue !== undefined) {
                                        configListModel.setProperty(index, "value", model.originalValue)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    footer: DialogButtonBox {
        Button {
            text: "Применить"
            enabled: isClientConnected
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            highlighted: true
            font.pixelSize: AppTheme.normalFontSize
        }

        Button {
            text: "Отмена"
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            font.pixelSize: AppTheme.normalFontSize
        }

        onAccepted: {
            if (!isClientConnected) {
                console.warn("Клиент не зарегистрирован")
                return
            }

            // Собираем конфигурацию
            var updatedConfig = {}

            for (var i = 0; i < configListModel.count; ++i) {
                var item = configListModel.get(i)
                if (item.key && !item.isReadOnly) {
                    updatedConfig[item.key] = item.value
                }
            }

            var message = {
                "descriptor":       clientData.descriptor,
                "id":               clientData.id,
                "allowSending":     allowSending,
                "type":             "Configuration",
                "payload":          updatedConfig
            }

            console.log("Отправка конфигурации:", JSON.stringify(message))
            configurationAccepted(message)

            configDialog.close()
        }

        onRejected: {
            configDialog.close()
        }
    }
}
