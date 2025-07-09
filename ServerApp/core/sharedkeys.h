/**
 * @file sharedkeys.h
 * @brief Определяет строковые ключи, используемые для обмена данными.
 */
#ifndef SHAREDKEYS_H
#define SHAREDKEYS_H

#include <QString>
#include <qobject.h>

#include "../common/protocol.h"

/**
 * @namespace Keys
 * @brief Пространство имен для константных строковых ключей.
 *
 * Эти ключи используются для единообразного доступа к данным в QVariantMap и JSON
 * объектах во всем приложении, предотвращая ошибки из-за опечаток.
 */
namespace Keys {
Q_NAMESPACE
// --- Ключи для UI (внутренние) ---
const QString CONFIGURATION = Protocol::MessageType::CONFIGURATION;

const QString ID            = Protocol::Keys::ID;
const QString TYPE          = Protocol::Keys::TYPE;
const QString PAYLOAD       = Protocol::Keys::PAYLOAD;

const QString DESCRIPTOR    = "descriptor";
const QString ADDRESS       = "address";
const QString PORT          = "port";
const QString STATUS        = "status";
const QString ALLOW_SENDING = "allowSending";
const QString TIME_STAMP    = "timestamp";
} // namespace Keys

#endif // SHAREDKEYS_H
