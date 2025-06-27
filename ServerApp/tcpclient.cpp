#include "tcpclient.h"

TcpClient::TcpClient(QTcpSocket *socket, QObject *parent)
    : IClient(parent), m_socket(socket), m_id(DEFAULT_ID) {

    Q_ASSERT(m_socket);

    if (m_socket->parent() == nullptr) {
        m_socket->setParent(this);
    }

    // Подключаем сигналы для отслеживания состояния
    connect(m_socket, &QTcpSocket::disconnected,
            this, &TcpClient::handleDisconnected);
    connect(m_socket, &QTcpSocket::readyRead,
            this, &TcpClient::handleReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred,
            this, &TcpClient::handleErrorOccurred);
}

quintptr TcpClient::descriptor() const {
    return m_socket ? m_socket->socketDescriptor() : 0;
}

bool TcpClient::isConnected() const {
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

QString TcpClient::peerAddress() const {
    return m_socket ?
               QHostAddress(m_socket->peerAddress().toIPv4Address()).toString() : QString();
}

quint16 TcpClient::peerPort() const {
    return m_socket ? m_socket->peerPort() : 0;
}

QAbstractSocket::SocketError TcpClient::error() const
{
    return m_socket ? m_socket->error() :
               QAbstractSocket::UnknownSocketError;
}

void TcpClient::id(const QString &id){
    m_id = id;
}

void TcpClient::connectToServer(const QString &host, quint16 port)
{
    if (m_socket->state() == QAbstractSocket::UnconnectedState) {
        m_socket->connectToHost(host, port);
    }
}

void TcpClient::disconnect()
{
    if (m_socket && isConnected()) {
        m_socket->disconnectFromHost();
    }
}

void TcpClient::sendData(const QByteArray &data) {
    if (m_socket && isConnected()) {
        m_socket->write(data);
    }
}

QByteArray TcpClient::readData() {
    return m_socket ? m_socket->readAll() : QByteArray();
}

void TcpClient::sendData(const QJsonObject &json) {
    if (m_socket && isConnected()) {
        QJsonDocument doc(json);
        m_socket->write(doc.toJson(QJsonDocument::Compact));
    }
}

QString TcpClient::id() const {
    return m_id;
}

void TcpClient::handleConnected() {
    emit connected();
}

void TcpClient::handleDisconnected() {
    emit disconnected();
}

void TcpClient::handleReadyRead() {
    if (m_socket && m_socket->bytesAvailable() > 0) {
        emit readyRead();
    }
}

void TcpClient::handleErrorOccurred() {
    emit errorOccurred();
}


