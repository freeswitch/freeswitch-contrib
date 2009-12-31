#include "call.h"

Call::Call()
{
}

Call::Call(int call_id, QString cid_name, QString cid_number, fsphone_call_direction_t direction, QString uuid) :
        _call_id(call_id),
        _cid_name(cid_name),
        _cid_number(cid_number),
        _direction(direction),
        _uuid (uuid)
{
}
