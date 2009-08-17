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
#include "eslconnection.h"

#define event_construct_common() event = NULL; serialized_string = NULL; mine = 0; hp = NULL

void eslSetLogLevel(int level)
{
    esl_global_set_default_logger(level);
}

ESLconnection::ESLconnection(const char *host, const char *port, const char *password, const char *name)
        : handle(NULL),_host(new QString(host)), _port(new QString(port)),
        _pass(new QString(password)), _name(new QString(name))
{
    qRegisterMetaType<ESLevent>("ESLevent");
}

ESLconnection::~ESLconnection()
{
    if (isConnected()) {

        {
            QMutexLocker locker(&_commandQueueMutex);
            while(!_commandQueue.isEmpty())
                _commandQueue.dequeue();
        }

        esl_disconnect(handle);
    }
    wait();
}

QString ESLconnection::getName()
{
    return *_name;
}

int ESLconnection::disconnect()
{
    if (handle)
    {
        if (handle->connected)
        {
            return esl_disconnect(handle);
        }
    }
    return 0;
}

bool ESLconnection::setConsoleLogLevel(int level)
{
    ESLevent *event = sendRecv(QString("log %1").arg(level).toAscii());
    if (event)
    {
        return true;
    }
    return false;
}

void ESLconnection::connect()
{
    start();
}

int ESLconnection::isConnected()
{
    if (handle)
        return handle->connected;
    return 0;
}

void ESLconnection::send(QString cmd)
{
    esl_send(handle, cmd.toAscii());
}

ESLevent *ESLconnection::sendRecv(const char *cmd)
{
    if (esl_send_recv(handle, cmd) == ESL_SUCCESS) {
        esl_event_t *event;
        esl_event_dup(&event, handle->last_sr_event);
        return new ESLevent(event);
    }

    return NULL;
}

/*
void ESLconnection::api(QString cmd)
{
    if (cmd.isEmpty()) {
        return;
    }

    if (esl_send_recv(handle, QString("api %1").arg(cmd).toAscii()) == ESL_SUCCESS) {
        esl_event_t *event;
        esl_event_dup(&event, handle->last_sr_event);
        emit gotEvent(new ESLevent(event, 1));
    }
}
*/

ESLevent * ESLconnection::bgapi(QString cmd)
{
    if (cmd.isEmpty()) {
        return NULL;
    }

    if (esl_send_recv(handle, QString("bgapi %1").arg(cmd).toAscii()) == ESL_SUCCESS) {
        esl_event_t *event;
        esl_event_dup(&event, handle->last_sr_event);
        return new ESLevent(event);;
    }
    return NULL;
}

ESLevent *ESLconnection::getInfo()
{
    if (handle->connected && handle->info_event) {
        esl_event_t *event;
        esl_event_dup(&event, handle->info_event);
        return new ESLevent(event);
    }

    return NULL;
}

int ESLconnection::setAsyncExecute(const char *val)
{
    if (val) {
        handle->async_execute = esl_true(val);
    }
    return handle->async_execute;
}

int ESLconnection::setEventLock(const char *val)
{
    if (val) {
        handle->event_lock = esl_true(val);
    }
    return handle->event_lock;
}

int ESLconnection::execute(const char *app, const char *arg, const char *uuid)
{
    return esl_execute(handle, app, arg, uuid);
}


int ESLconnection::executeAsync(const char *app, const char *arg, const char *uuid)
{
    int async = handle->async_execute;
    int r;

    handle->async_execute = 1;
    r = esl_execute(handle, app, arg, uuid);
    handle->async_execute = async;

    return r;
}

/*
int ESLconnection::sendEvent(ESLevent *send_me)
{
    return esl_sendevent(handle, send_me->event);
}
*/

ESLevent *ESLconnection::recvEvent()
{

    if (esl_recv_event(handle, 1, NULL) == ESL_SUCCESS) {
        esl_event_t *e = handle->last_ievent ? handle->last_ievent : handle->last_event;
        if (e) {
            esl_event_t *event;
            esl_event_dup(&event, e);
            return new ESLevent(event);
        }
    }

    //last_event_obj = new ESLevent("server_disconnected");

    return NULL;
}

ESLevent *ESLconnection::recvEventTimed(int ms)
{
    if (esl_recv_event_timed(handle, ms, 1, NULL) == ESL_SUCCESS) {
        esl_event_t *e = handle->last_ievent ? handle->last_ievent : handle->last_event;
        if (e) {
            esl_event_t *event;
            esl_event_dup(&event, e);
            return new ESLevent(event);
        }
    }

    return NULL;
}

ESLevent *ESLconnection::filter(const char *header, const char *value)
{
    esl_status_t status = esl_filter(handle, header, value);

    if (status == ESL_SUCCESS && handle->last_sr_event) {
        esl_event_t *event;
        esl_event_dup(&event, handle->last_sr_event);
        return new ESLevent(event);
    }

    return NULL;

}

int ESLconnection::events(const char *etype, const char *value)
{
    esl_event_type_t type_id = ESL_EVENT_TYPE_PLAIN;

    if (!strcmp(etype, "xml")) {
        type_id = ESL_EVENT_TYPE_XML;
    }

    return esl_events(handle, type_id, value);
}

void ESLconnection::addCommand(CommandTransaction *commandT)
{
    QMutexLocker locker(&_commandQueueMutex);
    _commandQueue.enqueue(commandT);
}

void ESLconnection::run(void)
{

    handle = new esl_handle_t;
    memset(handle, 0, sizeof(esl_handle_t));

    CommandTransaction *commandTransaction = 0;
    ESLevent * event = NULL;

    if (esl_connect(handle, _host->toAscii(), _port->toInt(), _pass->toAscii()) == ESL_SUCCESS)
    {
        events("plain", "all");
        send("log 7");
        emit connected();
    }

    while(isConnected())
    {
        // Process 1 received event
        event = recvEventTimed(10);
        if (event)
        {
            processReceivedEvent(event);
        }

        // Process all pending commands
        {
            QMutexLocker locker(&_commandQueueMutex);
            while (!_commandQueue.isEmpty())
            {
                commandTransaction = _commandQueue.dequeue();
                event = bgapi(commandTransaction->getCommand());
                qDebug() << "Sent a command and got this response : " << event;
                if (event)
                    _commandHash.insert(event->_headers.value("Job-UUID"), commandTransaction);
            }
        }
    }
    if (!QString(handle->err).isEmpty())
    {
        emit connectionFailed(handle->err);
    }
    else
    {
        emit disconnected();
    }
}

void ESLconnection::processReceivedEvent(ESLevent *event)
{
    CommandTransaction *commandTransaction = 0;
    /* Process all log messages */
    if (QString(event->_headers.value("Content-Type")) == QString("log/data"))
    {
        emit receivedLogMessage(*event);
    }
    /* Process command responses */
    if (event->_headers.value("Job-UUID") != NULL)
    {
        if(_commandHash.contains(event->_headers.value("Job-UUID")))
        {
            commandTransaction = _commandHash.value(event->_headers.value("Job-UUID"));
            commandTransaction->setResponse(*event);
        }
        else
        {
            qDebug() << "Something went wrong... We should log this...";
        }
    }
    /* Process all other events*/
    emit receivedChannelEvent(*event);
}

CommandTransaction::CommandTransaction(QString command)
{
    _command = command;
}

void CommandTransaction::setResponse(ESLevent e)
{
    emit gotResponse(e);
}
