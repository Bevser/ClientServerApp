import QtQuick

pragma Singleton

QtObject {
    // Основные цвета кнопок
    readonly property color toolButtonPressed:          "#d4d4d4"
    readonly property color toolButtonHovered:          "#e8e8e8"
    readonly property color toolButtonBorder:           "#c0c0c0"
    readonly property color toolButtonBorderPressed:    "#a0a0a0"
    readonly property color separatorColor:             "#d0d0d0"

    // Цвета кнопки "Запустить"
    readonly property color startButtonPressed:         "#c8e6c9"
    readonly property color startButtonHovered:         "#e8f5e8"
    readonly property color startButtonBorder:          "#81c784"
    readonly property color startButtonBorderPressed:   "#4caf50"
    readonly property color startButtonNormal:          "#66BB6A"
    readonly property color startButtonHover:           "#4CAF50"
    readonly property color startButtonPress:           "#388E3C"
    readonly property color startButtonBorderColor:     "#388E3C"

    // Цвета кнопки "Остановить"
    readonly property color stopButtonPressed:          "#ffcdd2"
    readonly property color stopButtonHovered:          "#ffebee"
    readonly property color stopButtonBorder:           "#e57373"
    readonly property color stopButtonBorderPressed:    "#f44336"
    readonly property color stopButtonNormal:           "#ff5722"
    readonly property color stopButtonHover:            "#f44336"
    readonly property color stopButtonPress:            "#d32f2f"
    readonly property color stopButtonBorderColor:      "#d32f2f"

    // Цвета кнопки "Очистить"
    readonly property color clearButtonNormal:          "#607D8B"
    readonly property color clearButtonHovered:         "#78909C"
    readonly property color clearButtonPressed:         "#546E7A"
    readonly property color clearButtonBorder:          "#455A64"

    // Цвета кнопки "Удалить"
    readonly property color deleteButtonNormal:         "#A1887F"
    readonly property color deleteButtonHover:          "#8D6E63"
    readonly property color deleteButtonPress:          "#795548"
    readonly property color deleteButtonBorderColor:    "#5D4037"

    // Цвета отключенных кнопок
    readonly property color disabledButtonBackground:   "#e0e0e0"
    readonly property color disabledButtonBorder:       "#bdbdbd"
    readonly property color disabledButtonText:         "#9e9e9e"

    // Цвета лога
    readonly property color logBackground:              "#fafafa"
    readonly property color logBorder:                  "#ddd"
    readonly property color logText:                    "#333"

    // Цвета для диалогов
    readonly property color headerBackground:           "#f8f9fa"
    readonly property color headerBorder:               "#e0e0e0"
    readonly property color secondaryText:              "#666"
    readonly property color connectedStatus:            "#4CAF50"
    readonly property color disconnectedStatus:         "#f44336"
    readonly property color fieldBackground:            "#ffffff"
    readonly property color fieldBorder:                "#e0e0e0"
    readonly property color readOnlyBackground:         "#fff3e0"
    readonly property color placeholderText:            "#999"

    // Цвета для таблиц
    readonly property color headerColor:                "#37474f"
    readonly property color headerHoverColor:           "#455a64"
    readonly property color headerBorderColor:          "#263238"
    readonly property color headerTextColor:            "white"
    readonly property color evenRowColor:               "#ffffff"
    readonly property color oddRowColor:                "#f8f9fa"
    readonly property color hoverRowColor:              "#f0f8f0"
    readonly property color selectedRowColor:           "#dcf0dc"
    readonly property color borderColor:                "#e0e0e0"
    readonly property color textColor:                  "#424242"

    // Цвета для подсказок
    readonly property color tooltipBackgroundColor:     "#2c2c2c"
    readonly property color tooltipBorderColor:         "#555555"
    readonly property color tooltipTextColor:           "#ffffff"

    // Шрифты
    readonly property string monoFont:                  "Consolas, Monaco, monospace"
    readonly property string fontFamily:                "Arial"

    // Размеры
    readonly property int toolButtonSize:               24
    readonly property int smallFontSize:                10
    readonly property int normalFontSize:               14
    readonly property int fontSize:                     11
    readonly property int rowHeight:                    35
    readonly property int headerHeight:                 35
}
