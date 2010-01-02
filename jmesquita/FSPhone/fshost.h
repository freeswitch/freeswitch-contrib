#ifndef FSHOST_H
#define FSHOST_H

#include <QThread>
#include <QHash>
#include <switch.h>

class Call;

#define FSPHONE_GW_STATE_TRYING 0
#define FSPHONE_GW_STATE_REGISTER 1
#define FSPHONE_GW_STATE_REGED 2
#define FSPHONE_GW_STATE_UNREGED 3
#define FSPHONE_GW_STATE_UNREGISTER 4
#define FSPHONE_GW_STATE_FAILED 5
#define FSPHONE_GW_STATE_FAIL_WAIT 6
#define FSPHONE_GW_STATE_EXPIRED 7
#define FSPHONE_GW_STATE_NOREG 8

static const char *fsphone_gw_state_names[] = {
    "TRYING",
    "REGISTER",
    "REGED",
    "UNREGED",
    "UNREGISTER",
    "FAILED",
    "FAIL_WAIT",
    "EXPIRED",
    "NOREG"
};

class FSHost : public QThread
{
Q_OBJECT
public:
    explicit FSHost(QObject *parent = 0);
    switch_status_t sendCmd(const char *cmd, const char *args, QString *res);
    void generalEventHandler(switch_event_t *event);
    Call * getCallByUUID(QString uuid) { return _active_calls.value(uuid, NULL); }
    QString getGwStateName(int id) { return fsphone_gw_state_names[id]; }

protected:
    void run(void);

signals:
    void coreLoadingError(QString);
    void ready(void);
    void ringing(QString);
    void answered(QString);
    void newOutgoingCall(QString);
    void hungup(QString);
    void gwStateChange(QString, int);

private:
    QHash<QString, Call*> _active_calls;
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
    switch_safe_free(clone);
}

#endif // FSHOST_H
