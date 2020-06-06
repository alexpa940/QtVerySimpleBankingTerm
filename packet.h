#ifndef PACKET_H
#define PACKET_H
#include <QtCore/qdatastream.h>
#include <QtCore/qdatetime.h>

class Packet
{
public:
    qint16 transaction_id;
    QString pan;
    QString userName;
    QString expiryDate;
    qint16 pin;
    float amount;
    QString terminal_id;
    qint16 responseCode;
    QDateTime dateTime;
};

QDataStream &operator>>(QDataStream &stream, Packet &pckt);



QDataStream &operator<<(QDataStream &stream, const Packet &pckt);

#endif // PACKET_H
