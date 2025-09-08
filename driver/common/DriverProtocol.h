#ifndef DRIVER_PROTOCOL_H
#define DRIVER_PROTOCOL_H

#include <QtGlobal>
#include <QByteArray>
#include <QDataStream>

namespace DriverProtocol {

// Mensagens suportadas entre Plugin <-> Driver
enum class MessageType : quint16 {
    Ping                = 1,
    Pong                = 2,
    SetLedColors        = 10, // payload: array de RGB (RGB888)
    SetBrightness       = 11, // payload: 1 float [0..1]
    GetStatus           = 20, // payload: vazio
    StatusResponse      = 21  // payload: string/JSON curto
};

// Cabeçalho binário (little-endian)
// [uint32 length][uint16 type]
// length = tamanho de (type + payload)
struct Header {
    quint32 length;   // bytes de type+payload
    quint16 type;     // MessageType
};

inline QByteArray pack(MessageType type, const QByteArray& payload)
{
    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);

    Header h;
    h.length = static_cast<quint32>(sizeof(quint16) + payload.size());
    h.type   = static_cast<quint16>(type);

    out << h.length;
    out << h.type;
    if (!payload.isEmpty()) {
        out.writeRawData(payload.constData(), payload.size());
    }
    return buffer;
}

inline bool tryUnpack(QByteArray& inoutBuffer, MessageType& outType, QByteArray& outPayload)
{
    // Necessário no mínimo o header
    if (inoutBuffer.size() < static_cast<int>(sizeof(quint32) + sizeof(quint16))) {
        return false;
    }

    QDataStream in(inoutBuffer);
    in.setByteOrder(QDataStream::LittleEndian);

    quint32 length = 0; // type + payload
    quint16 type   = 0;

    in >> length;
    in >> type;

    const int totalNeeded = static_cast<int>(sizeof(quint32) + sizeof(quint16) + length);
    if (inoutBuffer.size() < totalNeeded) {
        return false; // aguardar mais dados
    }

    outType = static_cast<MessageType>(type);
    const int payloadSize = static_cast<int>(length) - static_cast<int>(sizeof(quint16));
    if (payloadSize > 0) {
        outPayload = inoutBuffer.mid(sizeof(quint32) + sizeof(quint16), payloadSize);
    } else {
        outPayload.clear();
    }

    // Consumir do buffer
    inoutBuffer.remove(0, totalNeeded);
    return true;
}

} // namespace DriverProtocol

#endif // DRIVER_PROTOCOL_H
