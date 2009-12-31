#ifndef CALL_H
#define CALL_H

#include <QString>
#include <fshost.h>

typedef enum {
        FSPHONE_CALL_DIRECTION_INBOUND = 0,
        FSPHONE_CALL_DIRECTION_OUTBOUND = 1
} fsphone_call_direction_t;

class Call
{
public:
    Call(int call_id, QString cid_name, QString cid_number);

private:
    int _call_id;
    QString _cid_name;
    QString _cid_number;
    fsphone_call_direction_t _direction;
};

#endif // CALL_H
