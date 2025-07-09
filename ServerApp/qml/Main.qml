import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ServerApp

ApplicationWindow {
    id: root
    width:          1280
    height:         800
    minimumHeight:  600
    minimumWidth:   800
    visible:        true
    title:          "Базовая станция"
    color:          "#f0f0f0"

    // Проверка доступности viewModel
    readonly property bool hasViewModel: viewModel !== null && viewModel !== undefined
    readonly property bool hasClientModel: hasViewModel && viewModel.clientTableModel !== null
    readonly property bool hasDataModel: hasViewModel && viewModel.dataTableModel !== null

    // Диалоги
    ConfigurationDialog {
        id: configDialog
        anchors.centerIn: parent
        onConfigurationAccepted: function(message) {
            if (root.hasViewModel) {
                viewModel.updateClientConfiguration(message)
            }
        }
    }

    ServerManagementDialog {
        id: serverDialog
        anchors.centerIn: parent
        model: root.hasViewModel ? viewModel.serverListModel : null
    }

    // Компоненты для переиспользования
    Component {
        id: clearButtonComponent
        Button {
            property string buttonText: "Очистить"
            property var clickHandler: null

            text: buttonText
            highlighted: true
            onClicked: if (clickHandler) clickHandler()

            background: Rectangle {
                color: parent.down ? AppTheme.clearButtonPressed :
                      (parent.hovered ? AppTheme.clearButtonHovered : AppTheme.clearButtonNormal)
                radius: 3
                border.color: AppTheme.clearButtonBorder
            }

            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: AppTheme.smallFontSize
            }
        }
    }

    Component {
        id: toolButtonComponent
        ToolButton {
            property string iconPath: ""
            property string tooltipText: ""
            property var clickHandler: null
            property string buttonType: "normal" // normal, start, stop

            icon.width: AppTheme.toolButtonSize
            icon.height: AppTheme.toolButtonSize
            icon.source: iconPath
            display: AbstractButton.IconOnly
            onClicked: if (clickHandler) clickHandler()

            ToolTip.visible: hovered
            ToolTip.text: tooltipText

            background: Rectangle {
                color: {
                    if (buttonType === "start") {
                        return parent.pressed ? AppTheme.startButtonPressed :
                              (parent.hovered ? AppTheme.startButtonHovered : "transparent")
                    } else if (buttonType === "stop") {
                        return parent.pressed ? AppTheme.stopButtonPressed :
                              (parent.hovered ? AppTheme.stopButtonHovered : "transparent")
                    }
                    return parent.pressed ? AppTheme.toolButtonPressed :
                          (parent.hovered ? AppTheme.toolButtonHovered : "transparent")
                }
                radius: 4
                border.color: {
                    if (buttonType === "start") {
                        return parent.pressed ? AppTheme.startButtonBorderPressed :
                              (parent.hovered ? AppTheme.startButtonBorder : "transparent")
                    } else if (buttonType === "stop") {
                        return parent.pressed ? AppTheme.stopButtonBorderPressed :
                              (parent.hovered ? AppTheme.stopButtonBorder : "transparent")
                    }
                    return parent.pressed ? AppTheme.toolButtonBorderPressed :
                          (parent.hovered ? AppTheme.toolButtonBorder : "transparent")
                }
                border.width: 1
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Панель управления
        ToolBar {
            id: toolBar
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4

                // Кнопка управления серверами
                Loader {
                    sourceComponent: toolButtonComponent
                    onLoaded: {
                        item.iconPath = "qrc:/resource/settings.png"
                        item.tooltipText = "Менеджер серверов"
                        item.clickHandler = function() { serverDialog.open() }
                    }
                }

                // Разделитель
                Rectangle {
                    width: 1
                    height: AppTheme.toolButtonSize
                    color: AppTheme.separatorColor
                    Layout.alignment: Qt.AlignVCenter
                }

                // Кнопка запуска всех клиентов
                Loader {
                    sourceComponent: toolButtonComponent
                    onLoaded: {
                        item.iconPath = "qrc:/resource/start.png"
                        item.tooltipText = "Запустить всех клиентов"
                        item.buttonType = "start"
                        item.clickHandler = function() {
                            if (root.hasViewModel) viewModel.startAllClients()
                        }
                    }
                }

                // Кнопка остановки всех клиентов
                Loader {
                    sourceComponent: toolButtonComponent
                    onLoaded: {
                        item.iconPath = "qrc:/resource/stop.png"
                        item.tooltipText = "Остановить всех клиентов"
                        item.buttonType = "stop"
                        item.clickHandler = function() {
                            if (root.hasViewModel) viewModel.stopAllClients()
                        }
                    }
                }

                // Заполнитель
                Item {
                    Layout.fillWidth: true
                }
            }
        }

        // Основная область
        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            // Левая панель — клиенты
            Frame {
                SplitView.preferredWidth: 470
                SplitView.minimumWidth: 320

                Loader {
                    sourceComponent: clearButtonComponent
                    anchors.right: parent.right
                    onLoaded: {
                        item.buttonText = "Убрать отключенных"
                        item.clickHandler = function() {
                            if (root.hasViewModel) viewModel.removeDisconnectedClients()
                        }
                    }
                }

                UniversalTable {
                    id: clientTable
                    anchors.fill: parent
                    title: "Клиенты"
                    tableModel: root.hasClientModel ? viewModel.clientTableModel : null
                    columnWidths: root.hasClientModel ? viewModel.clientTableModel.columnWidths : []
                    columnHeaders: root.hasClientModel ? viewModel.clientTableModel.columnHeaders : []

                    onCellDoubleClicked: function(row, model) {
                        if (!root.hasClientModel) return
                        let rowData = viewModel.clientTableModel.getRowData(row)
                        if (rowData) {
                            configDialog.openWithData(rowData)
                        }
                    }

                    onHeaderClicked: function(column) {
                        if (root.hasViewModel) {
                            viewModel.sortClients(column)
                        }
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

                    Loader {
                        sourceComponent: clearButtonComponent
                        anchors.right: parent.right
                        onLoaded: {
                            item.clickHandler = function() {
                                if (root.hasViewModel) viewModel.clearData()
                            }
                        }
                    }

                    UniversalTable {
                        id: dataTable
                        anchors.fill: parent
                        title: "Данные"
                        tableModel: root.hasDataModel ? viewModel.dataTableModel : null
                        columnWidths: root.hasDataModel ? viewModel.dataTableModel.columnWidths : []
                        columnHeaders: root.hasDataModel ? viewModel.dataTableModel.columnHeaders : []
                        tooltipColumn: 3

                        onHeaderClicked: function(column) {
                            if (root.hasViewModel) {
                                viewModel.sortData(column)
                            }
                        }
                    }
                }

                // Нижняя часть — лог
                Frame {
                    SplitView.preferredHeight: 200
                    SplitView.minimumHeight: 100

                    Loader {
                        sourceComponent: clearButtonComponent
                        anchors.right: parent.right
                        onLoaded: {
                            item.clickHandler = function() {
                                if (root.hasViewModel) viewModel.clearLog()
                            }
                        }
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 5

                        Label {
                            text: "Лог"
                            font.bold: true
                            font.pixelSize: AppTheme.normalFontSize
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            TextArea {
                                id: logArea
                                text: root.hasViewModel ? viewModel.logText : ""
                                readOnly: true
                                wrapMode: TextArea.Wrap
                                font.family: AppTheme.monoFont
                                font.pixelSize: AppTheme.smallFontSize
                                color: AppTheme.logText
                                selectByMouse: true

                                background: Rectangle {
                                    color: AppTheme.logBackground
                                    border.color: AppTheme.logBorder
                                }

                                // Автоскролл к началу при обновлении
                                onTextChanged: cursorPosition = 0
                            }
                        }
                    }
                }
            }
        }
    }
}
