import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import enums 1.0

Dialog {
    id:     configDialog
    title:  "Конфигурация клиента"
    modal:  true
    width:  450
    height: 550

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

        readonly property color headerBackground:           "#f8f9fa"
        readonly property color headerBorder:               "#e0e0e0"
        readonly property color secondaryText:              "#666"
        readonly property color connectedStatus:            "#4CAF50"
        readonly property color disconnectedStatus:         "#f44336"
        readonly property color fieldBackground:            "#ffffff"
        readonly property color fieldBorder:                "#e0e0e0"
        readonly property color readOnlyBackground:         "#fff3e0"

        readonly property string monoFont:                  "Consolas, Monaco, monospace"
        readonly property int toolButtonSize:               24
        readonly property int smallFontSize:                10
        readonly property int normalFontSize:               14
    }

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
                    "originalValue": config[key] ? config[key].toString() : ""
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
        color: configDialog.theme.headerBackground
        border.color: configDialog.theme.headerBorder

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
                    font.pixelSize: configDialog.theme.normalFontSize
                }

                Label {
                    text: "Адрес: " + (clientData ? clientData.address : "N/A")
                    font.pixelSize: configDialog.theme.smallFontSize + 2
                    color: configDialog.theme.secondaryText
                }

                Label {
                    text: "Статус: " + (clientData ? AppEnums.clientStatusToString(clientData.status) : "N/A")
                    font.pixelSize: configDialog.theme.smallFontSize + 2
                    color: isClientConnected ? configDialog.theme.connectedStatus : configDialog.theme.disconnectedStatus
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
            font.pixelSize: configDialog.theme.normalFontSize

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Label {
                    text: allowSending ? "Разрешена" : "Запрещена"
                    color: allowSending ? configDialog.theme.connectedStatus : configDialog.theme.disconnectedStatus
                    font.bold: true
                    font.pixelSize: configDialog.theme.normalFontSize
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
            font.pixelSize: configDialog.theme.normalFontSize

            ScrollView {
                anchors.fill: parent
                clip: true

                ListView {
                    model: configListModel
                    spacing: 8

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 50
                        color: model.isReadOnly ? configDialog.theme.readOnlyBackground : configDialog.theme.fieldBackground
                        border.color: configDialog.theme.fieldBorder
                        radius: 4

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10

                            // Название параметра
                            Label {
                                text: model.key || ""
                                font.bold: true
                                font.pixelSize: configDialog.theme.normalFontSize
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
                                font.pixelSize: configDialog.theme.normalFontSize

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
                                font.pixelSize: configDialog.theme.normalFontSize

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
            font.pixelSize: configDialog.theme.normalFontSize
        }

        Button {
            text: "Отмена"
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            font.pixelSize: configDialog.theme.normalFontSize
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
