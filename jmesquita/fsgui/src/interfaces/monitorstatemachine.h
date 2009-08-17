#ifndef MONITORSTATEMACHINE_H
#define MONITORSTATEMACHINE_H

#include <QObject>
#include <QtCore>
#include "eslevent.h"

class ESLconnection;

class Channel
{
private:
    QString _uuid;
    QHash<QString, QString> _variables;
public:
    Channel() {}
    Channel(QString uuid, QHash<QString, QString> variables) : _uuid(uuid), _variables(variables) {}
    QHash<QString, QString> getVariables() { return _variables; }
    QString getUUID() { return _uuid; }
    QString getHeader(QString header) { return _variables.value(header, ""); }
    QHash<QString, QString> getHeaders() { return _variables; }
    void stateChanged(QHash<QString, QString> variables) { _variables = variables; }
};

class Call
{
private:
    QString _orig_uuid;
    Channel *_orig_channel;
    QString _caller_uuid;
    Channel *_caller_channel;
public:
    Call(QString orig_uuid, Channel *orig_channel,
         QString caller_uuid, Channel *caller_channel);
    QString getCallerUUID() { return _caller_uuid; }
    QString getOriginateeUUID() { return _orig_uuid; }
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
