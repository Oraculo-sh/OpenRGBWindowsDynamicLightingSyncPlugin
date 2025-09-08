#include "WDLDriverServer.h"

#include <QDebug>
#include <QFile>

using namespace DriverProtocol;

WDLDriverServer::WDLDriverServer(const QString& serverName, QObject* parent)
    : QObject(parent)
    , m_serverName(serverName)
{
    connect(&m_server, &QLocalServer::newConnection, this, &WDLDriverServer::onNewConnection);
}

WDLDriverServer::~WDLDriverServer()
{
    stop();
}

bool WDLDriverServer::start()
{
    // Remover servidor antigo se existir (ex: crash anterior)
    QLocalServer::removeServer(m_serverName);
    if (!m_server.listen(m_serverName)) {
        qWarning() << "WDLDriverServer: failed to listen on" << m_serverName << ":" << m_server.errorString();
        return false;
    }
    qInfo() << "WDLDriverServer listening on" << m_serverName;
    return true;
}

void WDLDriverServer::stop()
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.key()) {
            it.key()->disconnect(this);
            it.key()->close();
        }
    }
    m_clients.clear();

    if (m_server.isListening()) {
        m_server.close();
    }
    QLocalServer::removeServer(m_serverName);
}

void WDLDriverServer::onNewConnection()
{
    while (m_server.hasPendingConnections()) {
        QLocalSocket* sock = m_server.nextPendingConnection();
        if (!sock) continue;

        ClientCtx ctx;
        ctx.socket = sock;
        m_clients.insert(sock, ctx);

        connect(sock, &QLocalSocket::readyRead, this, &WDLDriverServer::onReadyRead);
        connect(sock, &QLocalSocket::errorOccurred, this, &WDLDriverServer::onSocketError);
        connect(sock, &QLocalSocket::disconnected, this, &WDLDriverServer::onDisconnected);

        emit clientConnected(QString::number(reinterpret_cast<quintptr>(sock)));
        qInfo() << "Client connected:" << sock;
    }
}

void WDLDriverServer::onReadyRead()
{
    QLocalSocket* sock = qobject_cast<QLocalSocket*>(sender());
    if (!sock || !m_clients.contains(sock)) return;

    ClientCtx& ctx = m_clients[sock];
    ctx.buffer.append(sock->readAll());

    processMessages(ctx);
}

void WDLDriverServer::onSocketError(QLocalSocket::LocalSocketError)
{
    QLocalSocket* sock = qobject_cast<QLocalSocket*>(sender());
    if (!sock) return;
    qWarning() << "Socket error:" << sock->errorString();
}

void WDLDriverServer::onDisconnected()
{
    QLocalSocket* sock = qobject_cast<QLocalSocket*>(sender());
    if (!sock) return;

    m_clients.remove(sock);
    emit clientDisconnected(QString::number(reinterpret_cast<quintptr>(sock)));
    sock->deleteLater();
    qInfo() << "Client disconnected:" << sock;
}

void WDLDriverServer::processMessages(ClientCtx& ctx)
{
    MessageType type;
    QByteArray payload;

    // Processa enquanto existir uma mensagem completa no buffer
    while (tryUnpack(ctx.buffer, type, payload)) {
        handleMessage(ctx.socket, type, payload);
    }
}

void WDLDriverServer::handleMessage(QLocalSocket* sock, MessageType type, const QByteArray& payload)
{
    switch (type) {
    case MessageType::Ping: {
        QByteArray pong = pack(MessageType::Pong, QByteArray());
        if (sock) sock->write(pong);
        break;
    }
    case MessageType::SetLedColors: {
        // TODO: integrar com driver virtual futuramente
        qInfo() << "Received SetLedColors with" << (payload.size() / 3) << "RGB triplets";
        break;
    }
    case MessageType::SetBrightness: {
        if (payload.size() >= static_cast<int>(sizeof(float))) {
            float value = 1.0f;
            memcpy(&value, payload.constData(), sizeof(float));
            qInfo() << "Received SetBrightness:" << value;
        }
        break;
    }
    case MessageType::GetStatus: {
        QByteArray status = QByteArray("{\"status\":\"ok\",\"connectedClients\":"
                                       + QByteArray::number(m_clients.size()) + "}");
        QByteArray msg = pack(MessageType::StatusResponse, status);
        if (sock) sock->write(msg);
        break;
    }
    default:
        qWarning() << "Unknown message type:" << static_cast<int>(type);
        break;
    }
}
