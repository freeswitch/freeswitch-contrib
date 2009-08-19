#ifndef MONITORSTATEMACHINE_H
#define MONITORSTATEMACHINE_H

#include <QObject>
#include <QtCore>
#include "eslevent.h"

class ESLconnection;

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
public:
    Channel(QString uuid, State *state);
    QHash<QString, QString> getCurrentStateVariables() { return _currentConfig->getHeaders(); }
    QString getUUID() { return *_uuid; }
    void setUUID(QString uuid) { delete _uuid; _uuid = new QString(uuid); }
    QString getCurrentStateHeader(QString header) { return _currentConfig->getHeader(header); }
    void stateChanged(State *state) { _states.append(state); _currentConfig = state; }
    QList <State *> getStates() { return _states; }
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
};

#endif // MONITORSTATEMACHINE_H
