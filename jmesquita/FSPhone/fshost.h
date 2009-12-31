#ifndef FSHOST_H
#define FSHOST_H

#include <QThread>
#include <switch.h>

class FSHost : public QThread
{
Q_OBJECT
public:
    explicit FSHost(QObject *parent = 0);
    switch_status_t sendCmd(const char *cmd, const char *args, QString *res);
    void generalEventHandler(switch_event_t *event);

protected:
    void run(void);

signals:
    void ready(void);
    void ringing(QString uuid, QString caller_id_name, QString caller_id_name);

};

extern FSHost g_FSHost;

/*
   Used to match callback from fs core. We dup the event and call the class
   method callback to make use of the signal/slot infrastructure.
*/
static void eventHandlerCallback(switch_event_t *event)
{
    switch_event_t *clone = NULL;
    if (switch_event_dup(&clone, event) == SWITCH_STATUS_SUCCESS) {
        g_FSHost.generalEventHandler(clone);
    }
    /*switch_safe_free(event);*/
}

#endif // FSHOST_H
