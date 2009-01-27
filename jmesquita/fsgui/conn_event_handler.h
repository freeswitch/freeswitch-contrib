#ifndef CONN_EVENT_HANDLER_H
#define CONN_EVENT_HANDLER_H

#include <QtGui>

#include "esl.h"
#include "esl_event.h"
#include "esl_threadmutex.h"
#include "esl_config.h"

class conn_event_handler : public QObject
{
    Q_OBJECT

public:
    conn_event_handler();
    ~conn_event_handler();
    void Connect(const QString&, int, const QString&);
    void Disconnect(void);
    bool isConnected(void) const;
private slots:
    void handleError();
    void readClient(void);
public slots:
    void sendMessage(const QString&);
private:    
    void handleRecvMessage(const QString&);
    esl_handle_t handle;
signals:
    void connectionError(const QString&);
    void onConnected(void);
    void messageSignal(const QString&);
};

#endif // CONN_EVENT_HANDLER_H
