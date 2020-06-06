#include "packet.h"

QDataStream &operator>>(QDataStream &stream, Packet &pckt)
{
    stream >> pckt.transaction_id;
    stream >> pckt.pan;
    stream >> pckt.userName;
    stream >> pckt.expiryDate;
    stream >> pckt.pin;
    stream >> pckt.amount;
    stream >> pckt.terminal_id;
    stream >> pckt.responseCode;
    stream >> pckt.dateTime;
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const Packet &pckt)
{
    stream << pckt.transaction_id;
    stream << pckt.pan;
    stream << pckt.userName;
    stream << pckt.expiryDate;
    stream << pckt.pin;
    stream << pckt.amount;
    stream << pckt.terminal_id;
    stream << pckt.responseCode;
    stream << pckt.dateTime;
    return stream;
}
