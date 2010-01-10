/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2009, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Joao Mesquita <jmesquita@freeswitch.org>
 *
 */
#ifndef CALL_H
#define CALL_H

#include <QtCore>
#include <QString>
#include <fshost.h>

typedef enum {
    FSCOMM_CALL_STATE_RINGING = 0,
    FSCOMM_CALL_STATE_TRYING  = 1,
    FSCOMM_CALL_STATE_ANSWERED = 2
} fscomm_call_state_t;

typedef enum {
        FSCOMM_CALL_DIRECTION_INBOUND = 0,
        FSCOMM_CALL_DIRECTION_OUTBOUND = 1
} fscomm_call_direction_t;

class Call
{
public:
    Call(void);
    Call(int call_id, QString cid_name, QString cid_number, fscomm_call_direction_t direction, QString uuid);
    QString getCidName(void) { return _cid_name; }
    QString getCidNumber(void) { return _cid_number; }
    int getCallID(void) { return _call_id; }
    QString getUUID(void) { return _uuid; }
    void setbUUID(QString uuid) { _buuid = uuid; }
    fscomm_call_direction_t getDirection() { return _direction; }
    fscomm_call_state_t getState() { return _state; }
    void setState(fscomm_call_state_t state) { _state = state; }

private:
    int _call_id;
    QString _cid_name;
    QString _cid_number;
    fscomm_call_direction_t _direction;
    QString _uuid;
    QString _buuid;
    fscomm_call_state_t _state;
};

Q_DECLARE_METATYPE(Call)

#endif // CALL_H
