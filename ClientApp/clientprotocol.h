/**
 * @file clientprotocol.h
 * @brief Определяет константы, специфичные для внутренней логики клиента.
 */
#ifndef CLIENTPROTOCOL_H
#define CLIENTPROTOCOL_H

#include <QString>

/**
 * @namespace Protocol
 * @brief Расширение основного пространства имен Protocol для констант клиента.
 */
namespace Protocol {

/**
 * @namespace Keys
 * @brief Ключи, используемые в JSON-объектах полезной нагрузки (payload).
 */
namespace Keys {
// --- Ключи конфигурации и метрик ---
const QString MAX_CPU_TEMP      = "maxCpuTemp";
const QString MAX_CPU_USAGE     = "maxCpuUsage";
const QString MAX_MEMORY_USAGE  = "maxMemoryUsage";
const QString MAX_BAND_WIDTH    = "maxBandWidth";
const QString MAX_LATENCY       = "maxLatency";
const QString MAX_PACKET_LOSS   = "maxPacketLoss";

// --- Ключи телеметрии ---
const QString BAND_WIDTH    = "bandWidth";
const QString LATENCY       = "latency";
const QString PACKET_LOSS   = "packetLoss";
const QString UP_TIME       = "upTime";
const QString CPU_USAGE     = "cpuUsage";
const QString MEMORY_USAGE  = "memoryUsage";
const QString CPU_TEMP      = "cpuTemp";
const QString JUNK          = "junk";
const QString SEVERITY      = "severity";
const QString MESSAGE       = "message";
}

/**
 * @namespace Severity
 * @brief Уровни критичности для лог-сообщений.
 */
namespace Severity {
const QString INFO      = "INFO";
const QString WARN      = "WARN";
const QString ERROR     = "ERROR";
const QString CRITICAL  = "CRITICAL";
}

/**
 * @namespace Messages
 * @brief Предопределенные тексты сообщений.
 */
namespace Messages {
const QString HIGH_CPU_TEMP         = "High CPU temperature detected";
const QString CONFIG_UPDATED        = "Configuration updated successfully";
const QString DB_CONNECT_FAILED     = "Failed to connect to database.";
const QString THRESHOLD_EXCEEDED    = "Превышены пороговые значения: ";
}

/**
 * @namespace LogMessages
 * @brief Стандартизированные сообщения для вывода в консоль клиента.
 */
namespace LogMessages {
const QString CLIENT_STARTING       = "[INFO] Client is starting...";
const QString CONNECTION_ATTEMPT    = "[INFO] Attempting to connect to %1:%2...";
const QString CONNECTED_SUCCESS     = "[OK] Successfully connected to server.";
const QString CONNECTION_CONFIRMED  = "[OK] Connection confirmed. My ID:";
const QString WAITING_START         = "[INFO] Waiting for 'start' command from server...";
const QString START_RECEIVED        = "[START] 'start' command received. Starting data transmission.";
const QString STOP_RECEIVED         = "[STOP] 'stop' command received. Stopping data transmission.";
const QString CONFIG_RECEIVED       = "[CONFIG] New configuration received from server.";
const QString CONFIG_PARAM          = "[CONFIG]";
const QString DISCONNECTED          = "[ERROR] Connection to server lost.";
const QString SOCKET_ERROR          = "[ERROR] Socket error:";
const QString INVALID_JSON          = "[ERROR] Invalid JSON from server:";
}

/**
 * @namespace Constants
 * @brief Общие константы, используемые в логике клиента.
 */
namespace Constants {
const QString CLIENT_ID     = "Client";         ///< Базовый ID, запрашиваемый при регистрации
const int DATA_TYPES_COUNT  = 3;                ///< Количество типов данных для циклического переключения
const int RECONNECT_TIMEOUT = 5000;             ///< Таймаут переподключения (мс)
const int MIN_DELAY         = 100;              ///< Минимальная задержка отправки данных (мс)
const int MAX_DELAY         = 1000;             ///< Максимальная задержка отправки данных (мс)
const int JUNK_LENGTH       = 200;              ///< Длина "мусорных" данных в логах
const QString JUNK_CHARS    = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
}

} // namespace Protocol

#endif // CLIENTPROTOCOL_H
