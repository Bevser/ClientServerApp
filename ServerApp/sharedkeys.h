#ifndef SHAREDKEYS_H
#define SHAREDKEYS_H

#include <QString>
#include <qobject.h>
#include "protocol.h"

namespace Keys {
Q_NAMESPACE
// --- Ключи для UI (внутренние) ---
const QString CONFIGURATION       = Protocol::MessageType::CONFIGURATION;

const QString ID                  = Protocol::Keys::ID;
const QString TYPE                = Protocol::Keys::TYPE;
const QString PAYLOAD             = Protocol::Keys::PAYLOAD;

const QString DESCRIPTOR          = "descriptor";
const QString ADDRESS             = "address";
const QString PORT                = "port";
const QString STATUS              = "status";
const QString ALLOW_SENDING       = "allowSending";
const QString TIME_STAMP          = "timestamp";
}


#endif // SHAREDKEYS_H
