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
 * Joao Mesquita <jmesquita (at) freeswitch.org>
 *
 */
#ifndef MONITORSTATEMACHINE_H
#define MONITORSTATEMACHINE_H

#include <QObject>
#include <QtCore>
#include "eslevent.h"

class ESLconnection;

class Event
{
private:
    QHash<QString, QString> _headers;
public:
    Event(QHash<QString, QString> headers);
    QString getEventName() { return _headers.value("Event-Name"); }
    QHash<QString, QString> getHeaders() { return _headers; }
};


class State
{
private:
    int _state_id;
    QString _state_name;
    QHash<QString, QString> _variables;
public:
    State(int state_id, QString state_name, QHash<QString, QString> variables);
    QHash<QString, QString> getHeaders() { return _variables; }
    QString getHeader(QString header) { return _variables.value(header, NULL); }
    QString getStateName() { return _state_name; }
    int getStateId() { return _state_id; }
};

class Channel
{
private:
    QString *_uuid;
    QList <State *> _states;
    State *_currentConfig;
    QList<Event *> _events;
public:
    Channel(QString uuid, State *state);
    QHash<QString, QString> getCurrentStateVariables() { return _currentConfig->getHeaders(); }
    QString getUUID() { return *_uuid; }
    void setUUID(QString uuid) { delete _uuid; _uuid = new QString(uuid); }
    QString getCurrentStateHeader(QString header) { return _currentConfig->getHeader(header); }
    void stateChanged(State *state) { _states.append(state); _currentConfig = state; }
    QList <State *> getStates() { return _states; }
    void addEvent(Event * event) { _events.append(event); }
    QList<Event *> getEvents() { return _events; }
    Event * getEvent(int i) { return _events.at(i); }
};

class Call
{
private:
    Channel *_orig_channel;
    Channel *_caller_channel;
public:
    Call(Channel *orig_channel, Channel *caller_channel);
    QString getCallerUUID() { return _caller_channel->getUUID(); }
    QString getOriginateeUUID() { return _orig_channel->getUUID(); }
    Channel *getCallerChannel() { return _caller_channel; }
    Channel *getOriginateeChannel() { return _orig_channel; }
};

class MonitorStateMachine : public QObject
{
    Q_OBJECT
private:
    QList<ESLconnection *> _listESL;
    QHash <QString, Channel *> _channels;
    QHash <QString, Channel *> _inactive_channels;
    /*! Always indexed by caller UUID */
    QHash <QString, Call *> _calls;
    /*! Always indexed by caller UUID */
    QHash <QString, Call *> _inactive_calls;
public:
    MonitorStateMachine();
    void addESLconnection(ESLconnection *);
    void delESLconnection(ESLconnection *);
    int channelCount() { return _channels.count(); }
    QList<Channel *> getChannels() { return _channels.values(); }
    QList<Channel *> getInactiveChannels() { return _inactive_channels.values(); }
    QList<Call *> getCalls() { return _calls.values(); }
    QList<Call *> getInactiveCalls() { return _inactive_calls.values(); }
    Channel * getChannel(QString uuid) { return _channels.value(uuid, NULL); }
    Channel * getInactiveChannel(QString uuid) { return _inactive_channels.value(uuid, NULL); }
    Call * getCall(QString uuid) { return _calls.value(uuid, NULL); }
    Call * getInactiveCall(QString uuid) { return _inactive_calls.value(uuid, NULL); }
private slots:
    void processEvent(ESLevent);
signals:
    void channelStateChanged(Channel *);
    void channelCreated(Channel *);
    void callCreated(Call *);
    void callDestroyed(Call *);
    void channelDestroyed(Channel *);
    void newEvent(Channel *, Event *);
};

#endif // MONITORSTATEMACHINE_H
