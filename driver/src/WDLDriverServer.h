#ifndef WDL_DRIVER_SERVER_H
#define WDL_DRIVER_SERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>
#include <QHash>
#include <QByteArray>
#include <QTimer>
#include <QElapsedTimer>

#include "../common/DriverProtocol.h"

class WDLDriverServer : public QObject
{
    Q_OBJECT
public:
    explicit WDLDriverServer(const QString& serverName, QObject* parent = nullptr);
    ~WDLDriverServer();

    bool start();
    void stop();

signals:
    void clientConnected(const QString& id);
    void clientDisconnected(const QString& id);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onSocketError(QLocalSocket::LocalSocketError);
    void onDisconnected();

private:
    struct ClientCtx {
        QPointer<QLocalSocket> socket;
        QByteArray buffer; // acumula dados para framing
    };

    QString m_serverName;
    QLocalServer m_server;
    QHash<QLocalSocket*, ClientCtx> m_clients;

    void processMessages(ClientCtx& ctx);
    void handleMessage(QLocalSocket* sock, DriverProtocol::MessageType type, const QByteArray& payload);
};

#endif // WDL_DRIVER_SERVER_H
