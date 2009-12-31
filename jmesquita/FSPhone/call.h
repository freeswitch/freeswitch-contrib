#ifndef CALL_H
#define CALL_H

#include <QtCore>
#include <QString>
#include <fshost.h>

typedef enum {
        FSPHONE_CALL_DIRECTION_INBOUND = 0,
        FSPHONE_CALL_DIRECTION_OUTBOUND = 1
} fsphone_call_direction_t;

class Call
{
public:
    Call(void);
    Call(int call_id, QString cid_name, QString cid_number, fsphone_call_direction_t direction, QString uuid);
    QString getCidName(void) { return _cid_name; }
    QString getCidNumber(void) { return _cid_number; }
    int getCallID(void) { return _call_id; }
    QString getUUID(void) { return _uuid; }

private:
    int _call_id;
    QString _cid_name;
    QString _cid_number;
    fsphone_call_direction_t _direction;
    QString _uuid;
};

Q_DECLARE_METATYPE(Call)

#endif // CALL_H
