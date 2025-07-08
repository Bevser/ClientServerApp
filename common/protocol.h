#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>

namespace Protocol {

// Типы сообщений
namespace MessageType {
// --- От клиента к серверу ---
const QString REGISTRATION = "Registration";
const QString NETWORK_METRICS = "NetworkMetrics";
const QString DEVICE_STATUS = "DeviceStatus";
const QString LOG = "Log";

// --- От сервера к клиенту ---
const QString CONFIRMATION = "Confirmation";
const QString CONFIGURATION = "Configuration";
const QString COMMAND = "Command";
} // namespace MessageType

// Ключи, используемые в JSON-объектах
namespace Keys {
const QString ID = "id";
const QString TYPE = "type";
const QString PAYLOAD = "payload";
const QString COMMAND = "command";
} // namespace Keys

// Команды, которые сервер может отправлять клиенту
namespace Commands {
const QString START = "start";
const QString STOP = "stop";
} // namespace Commands
} // namespace Protocol

#endif // PROTOCOL_H
