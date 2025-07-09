#include "tcpclient.h"

// Конструктор
TcpClient::TcpClient(QTcpSocket *socket, QObject *parent)
    : IClient(parent), m_socket(socket), m_id(""),
    m_descriptor(socket->socketDescriptor()) {
    if (!m_socket)
        return;

    if (m_socket->parent() == nullptr) {
        m_socket->setParent(this);
    }

    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::handleConnected);
    connect(m_socket, &QTcpSocket::disconnected, this,
            &TcpClient::handleDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::handleReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpClient::handleError);
}

quintptr TcpClient::descriptor() const { return m_socket ? m_descriptor : 0; }

bool TcpClient::isConnected() const {
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

QString TcpClient::address() const {
    return m_socket
               ? QHostAddress(m_socket->peerAddress().toIPv4Address()).toString()
               : "0";
}

quint16 TcpClient::port() const { return m_socket ? m_socket->peerPort() : 0; }

void TcpClient::setId(const QString &id) { m_id = id; }

QString TcpClient::id() const { return m_id; }

void TcpClient::connectToHost(const QString &host, quint16 port) {
    if (m_socket && m_socket->state() == QAbstractSocket::UnconnectedState) {
        m_socket->connectToHost(host, port);
    }
}

void TcpClient::disconnect() {
    if (isConnected()) {
        m_socket->disconnectFromHost();
    }
}

void TcpClient::sendData(const QByteArray &data) {
    if (isConnected()) {
        m_socket->write(data);
    }
}

void TcpClient::sendData(const QJsonObject &json) {
    if (isConnected()) {
        QJsonDocument doc(json);
        m_socket->write(doc.toJson(QJsonDocument::Compact));
    }
}

void TcpClient::handleConnected() { emit connected(); }

void TcpClient::handleDisconnected() { emit disconnected(); }

void TcpClient::handleReadyRead() {
    QByteArray data = m_socket->readAll();
    emit dataReceived(data);
}

void TcpClient::handleError(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError)
    emit errorOccurred(m_socket->errorString());
}


