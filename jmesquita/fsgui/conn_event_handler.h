#ifndef CONN_EVENT_HANDLER_H
#define CONN_EVENT_HANDLER_H

#include <QtGui>

extern "C" {
    #include "esl.h"
    #include "esl_event.h"
    #include "esl_threadmutex.h"
    #include "esl_config.h"
}


class conn_event_handler : public QObject
{

    Q_OBJECT

public:
    conn_event_handler();
    ~conn_event_handler();
    void getConnected(QString, int, QString);
    void getDisconnected(void);
    bool isConnected(void);
private slots:
    void handleError();
    void readClient(void);
public slots:
    void sendMessage(QString);
private:    
    void handleRecvMessage(QString);
    esl_handle_t handle;
signals:
    void connectionError(QString);
    void gotConnected(void);
    void messageSignal(QString);
};

#endif // CONN_EVENT_HANDLER_H
