/**
 * @file protocol.h
 * @brief Определяет константы протокола обмена данными между клиентом и сервером.
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>

/**
 * @namespace Protocol
 * @brief Пространство имен для всех констант, связанных с протоколом.
 */
namespace Protocol {

/**
 * @namespace MessageType
 * @brief Типы сообщений, которыми обмениваются клиент и сервер.
 */
namespace MessageType {
// --- От клиента к серверу ---
const QString REGISTRATION      = "Registration";   ///< Регистрация клиента на сервере.
const QString NETWORK_METRICS   = "NetworkMetrics"; ///< Отправка метрик сети.
const QString DEVICE_STATUS     = "DeviceStatus";   ///< Отправка статуса устройства.
const QString LOG               = "Log";            ///< Отправка логов.

// --- От сервера к клиенту ---
const QString CONFIRMATION      = "Confirmation";   ///< Подтверждение регистрации.
const QString CONFIGURATION     = "Configuration";  ///< Отправка конфигурации клиенту.
const QString COMMAND           = "Command";        ///< Отправка команды клиенту.
} // namespace MessageType

/**
 * @namespace Keys
 * @brief Ключи, используемые в JSON-объектах сообщений.
 */
namespace Keys {
const QString ID                = "id";             ///< Идентификатор клиента.
const QString TYPE              = "type";           ///< Тип сообщения (из MessageType).
const QString PAYLOAD           = "payload";        ///< Полезная нагрузка (данные).
const QString COMMAND           = "command";        ///< Текст команды
} // namespace Keys

/**
 * @namespace Commands
 * @brief Команды, которые сервер может отправлять клиенту.
 */
namespace Commands {
const QString START             = "start";          ///< Команда на запуск
const QString STOP              = "stop";           ///< Команда на остановку
} // namespace Commands
} // namespace Protocol

#endif // PROTOCOL_H
