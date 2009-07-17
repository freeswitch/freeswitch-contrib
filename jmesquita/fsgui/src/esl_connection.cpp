/*
 * Copyright (c) 2007, Anthony Minessale II
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributor(s):
 *
 * Joao Mesquita <jmesquita (at) gmail.com>
 *
 */

#include <QtGui>
#include <esl.h>
#include <esl_connection.h>
#include "global_defines.h"

#define connection_construct_common() memset(&handle, 0, sizeof(handle)); last_event_obj = NULL
#define event_construct_common() event = NULL; serialized_string = NULL; mine = 0; hp = NULL

void eslSetLogLevel(int level)
{
    esl_global_set_default_logger(level);
}

ESLconnection::ESLconnection(const char *host, const char *port, const char *password)
        : _host(new QString(host)), _port(new QString(port)), _pass(new QString(password))
{
    connection_construct_common();
}

ESLconnection::~ESLconnection()
{
    if (handle.connected) {
        esl_disconnect(&handle);
    }

}

int ESLconnection::disconnect()
{
    if (handle.connected) {
        return esl_disconnect(&handle);
    }

    return 0;
}

void ESLconnection::doConnect()
{
    int x_port = _port->toInt();

    if (esl_connect(&handle, _host->toAscii(), x_port, _pass->toAscii()) == ESL_SUCCESS)
    {
        emit gotConnected();
    }
    start();
}

int ESLconnection::connected()
{
    return handle.connected;
}

int ESLconnection::send(const char *cmd)
{
    return esl_send(&handle, cmd);
}

ESLevent *ESLconnection::sendRecv(const char *cmd)
{
    if (esl_send_recv(&handle, cmd) == ESL_SUCCESS) {
        esl_event_t *event;
        esl_event_dup(&event, handle.last_sr_event);
        return new ESLevent(event, 1);
    }

    return NULL;
}

ESLevent *ESLconnection::api(const char *cmd, const char *arg)
{
    size_t len;
    char *cmd_buf;

    if (!cmd) {
        return NULL;
    }

    len = strlen(cmd) + (arg ? strlen(arg) : 0) + 10;

    cmd_buf = (char *) malloc(len + 1);
    assert(cmd_buf);

    snprintf(cmd_buf, len, "api %s %s", cmd, arg ? arg : "");
    *(cmd_buf + (len)) = '\0';


    if (esl_send_recv(&handle, cmd_buf) == ESL_SUCCESS) {
        esl_event_t *event;
        esl_event_dup(&event, handle.last_sr_event);
        return new ESLevent(event, 1);
    }

    free(cmd_buf);

    return NULL;
}

ESLevent *ESLconnection::bgapi(const char *cmd, const char *arg)
{
    size_t len;
    char *cmd_buf;

    if (!cmd) {
        return NULL;
    }

    len = strlen(cmd) + (arg ? strlen(arg) : 0) + 10;

    cmd_buf = (char *) malloc(len + 1);
    assert(cmd_buf);

    snprintf(cmd_buf, len, "bgapi %s %s", cmd, arg ? arg : "");
    *(cmd_buf + (len)) = '\0';

    if (esl_send_recv(&handle, cmd_buf) == ESL_SUCCESS) {
        esl_event_t *event;
        esl_event_dup(&event, handle.last_sr_event);
        return new ESLevent(event, 1);
    }

    free(cmd_buf);

    return NULL;
}

ESLevent *ESLconnection::getInfo()
{
    if (handle.connected && handle.info_event) {
        esl_event_t *event;
        esl_event_dup(&event, handle.info_event);
        return new ESLevent(event, 1);
    }

    return NULL;
}

int ESLconnection::setAsyncExecute(const char *val)
{
    if (val) {
        handle.async_execute = esl_true(val);
    }
    return handle.async_execute;
}

int ESLconnection::setEventLock(const char *val)
{
    if (val) {
        handle.event_lock = esl_true(val);
    }
    return handle.event_lock;
}

int ESLconnection::execute(const char *app, const char *arg, const char *uuid)
{
    return esl_execute(&handle, app, arg, uuid);
}


int ESLconnection::executeAsync(const char *app, const char *arg, const char *uuid)
{
    int async = handle.async_execute;
    int r;

    handle.async_execute = 1;
    r = esl_execute(&handle, app, arg, uuid);
    handle.async_execute = async;

    return r;
}

int ESLconnection::sendEvent(ESLevent *send_me)
{
    return esl_sendevent(&handle, send_me->event);
}

ESLevent *ESLconnection::recvEvent()
{
    if (last_event_obj) {
        delete last_event_obj;
    }

    if (esl_recv_event(&handle, 1, NULL) == ESL_SUCCESS) {
        esl_event_t *e = handle.last_ievent ? handle.last_ievent : handle.last_event;
        if (e) {
            esl_event_t *event;
            esl_event_dup(&event, e);
            last_event_obj = new ESLevent(event, 1);
            return last_event_obj;
        }
    }

    last_event_obj = new ESLevent("server_disconnected");

    return last_event_obj;
}

ESLevent *ESLconnection::recvEventTimed(int ms)
{
    if (last_event_obj) {
        delete last_event_obj;
        last_event_obj = NULL;
    }

    if (esl_recv_event_timed(&handle, ms, 1, NULL) == ESL_SUCCESS) {
        esl_event_t *e = handle.last_ievent ? handle.last_ievent : handle.last_event;
        if (e) {
            esl_event_t *event;
            esl_event_dup(&event, e);
            last_event_obj = new ESLevent(event, 1);
            return last_event_obj;
        }
    }

    return NULL;
}

ESLevent *ESLconnection::filter(const char *header, const char *value)
{
    esl_status_t status = esl_filter(&handle, header, value);

    if (status == ESL_SUCCESS && handle.last_sr_event) {
        esl_event_t *event;
        esl_event_dup(&event, handle.last_sr_event);
        return new ESLevent(event, 1);
    }

    return NULL;

}

int ESLconnection::events(const char *etype, const char *value)
{
    esl_event_type_t type_id = ESL_EVENT_TYPE_PLAIN;

    if (!strcmp(etype, "xml")) {
        type_id = ESL_EVENT_TYPE_XML;
    }

    return esl_events(&handle, type_id, value);
}

void ESLconnection::run(void)
{

    while(connected())
    {
        ESLevent * event = recvEventTimed(10);
        if (event)
        {
            QString type (event->getHeader("Content-Type"));
            if (QString::compare("log/data", type, Qt::CaseInsensitive) == 0)
            {
                ESLeventLog *e = new ESLeventLog(event);
                e->readSettings();
                emit gotConsoleEvent(e);
            }
            else
            {
                ESLevent * e = new ESLevent(event);
                emit gotEvent(e);
            }
        }
    }
    if (!QString(handle.err).isEmpty())
    {
        emit connectionFailed(handle.err);
    }
    else
    {
        emit gotDisconnected();
    }
}

// ESLevent
///////////////////////////////////////////////////////////////////////

ESLevent::ESLevent(const char *type, const char *subclass_name)
{
    esl_event_types_t event_id;

    event_construct_common();

    if (esl_name_event(type, &event_id) != ESL_SUCCESS) {
        event_id = ESL_EVENT_MESSAGE;
    }

    if (!esl_strlen_zero(subclass_name) && event_id != ESL_EVENT_CUSTOM) {
        esl_log(ESL_LOG_WARNING, "Changing event type to custom because you specified a subclass name!\n");
        event_id = ESL_EVENT_CUSTOM;
    }

    if (esl_event_create_subclass(&event, event_id, subclass_name) != ESL_SUCCESS) {
        esl_log(ESL_LOG_ERROR, "Failed to create event!\n");
        event = NULL;
    }

    serialized_string = NULL;
    mine = 1;
}

ESLevent::ESLevent(esl_event_t *wrap_me, int free_me)
{
    event_construct_common();
    event = wrap_me;
    mine = free_me;
    serialized_string = NULL;
}


ESLevent::ESLevent(ESLevent *me)
{
    /* workaround for silly php thing */
    event = me->event;
    mine = me->mine;
    serialized_string = NULL;
    me->event = NULL;
    me->mine = 0;
    esl_safe_free(me->serialized_string);
}

ESLevent::~ESLevent()
{

    if (serialized_string) {
        free(serialized_string);
    }

    if (event && mine) {
        esl_event_destroy(&event);
    }
}

const char *ESLevent::nextHeader(void)
{
    const char *name = NULL;

    if (hp) {
        name = hp->name;
        hp = hp->next;
    }

    return name;
}

const char *ESLevent::firstHeader(void)
{
    if (event) {
        hp = event->headers;
    }

    return nextHeader();
}

const char *ESLevent::serialize()
{
    this_check("");

    esl_safe_free(serialized_string);

    if (!event) {
        return "";
    }

    if (esl_event_serialize(event, &serialized_string, ESL_TRUE) == ESL_SUCCESS) {
        return serialized_string;
    }

    return "";

}

bool ESLevent::setPriority(esl_priority_t priority)
{
    this_check(false);

    if (event) {
        esl_event_set_priority(event, priority);
        return true;
    } else {
        esl_log(ESL_LOG_ERROR, "Trying to setPriority an event that does not exist!\n");
    }
    return false;
}

const char *ESLevent::getHeader(const char *header_name)
{
    this_check("");

    if (event) {
        return esl_event_get_header(event, header_name);
    } else {
        esl_log(ESL_LOG_ERROR, "Trying to getHeader an event that does not exist!\n");
    }
    return NULL;
}

bool ESLevent::addHeader(const char *header_name, const char *value)
{
    this_check(false);

    if (event) {
        return esl_event_add_header_string(event, ESL_STACK_BOTTOM, header_name, value) == ESL_SUCCESS ? true : false;
    } else {
        esl_log(ESL_LOG_ERROR, "Trying to addHeader an event that does not exist!\n");
    }

    return false;
}

bool ESLevent::delHeader(const char *header_name)
{
    this_check(false);

    if (event) {
        return esl_event_del_header(event, header_name) == ESL_SUCCESS ? true : false;
    } else {
        esl_log(ESL_LOG_ERROR, "Trying to delHeader an event that does not exist!\n");
    }

    return false;
}


bool ESLevent::addBody(const char *value)
{
    this_check(false);

    if (event) {
        return esl_event_add_body(event, "%s", value) == ESL_SUCCESS ? true : false;
    } else {
        esl_log(ESL_LOG_ERROR, "Trying to addBody an event that does not exist!\n");
    }

    return false;
}

char *ESLevent::getBody(void)
{

    this_check((char *)"");

    if (event) {
        return esl_event_get_body(event);
    } else {
        esl_log(ESL_LOG_ERROR, "Trying to getBody an event that does not exist!\n");
    }

    return NULL;
}

const char *ESLevent::getType(void)
{
    this_check("");

    if (event) {
        return esl_event_name(event->event_id);
    } else {
        esl_log(ESL_LOG_ERROR, "Trying to getType an event that does not exist!\n");
    }

    return (char *) "invalid";
}

/* ESLeventLog */
/***********************************************************************************************/
ESLeventLog::ESLeventLog(ESLevent *e) : ESLevent(e){}
ESLeventLog::~ESLeventLog(){}
void ESLeventLog::readSettings()
{
    QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);
    QColor consoleColor = settings.value(QString("Log-Level-%1-Color").arg(ESLevent::getHeader("Log-Level")), QColor(Qt::black)).value<QColor>();
}
QColor ESLeventLog::getConsoleColor()
{
    qDebug() << consoleColor;
    return consoleColor;
}
