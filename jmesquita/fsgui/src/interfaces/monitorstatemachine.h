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
    void stateChanged(QHash<QString, QString> variables) { _variables = variables; }
};

class MonitorStateMachine : public QObject
{
    Q_OBJECT
private:
    QList<ESLconnection *> _listESL;
    QHash <QString, Channel *> _channels;
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
    void channelDestroyed(Channel *);
};

#endif // MONITORSTATEMACHINE_H
