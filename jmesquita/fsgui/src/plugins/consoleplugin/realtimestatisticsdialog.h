#ifndef REALTIMESTATISTICSDIALOG_H
#define REALTIMESTATISTICSDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class RealtimeStatisticsDialog;
}

class QListWidgetItem;
class MonitorStateMachine;
class Channel;
class Call;

class RealtimeStatisticsDialog : public QDialog {
    Q_OBJECT
public:
    RealtimeStatisticsDialog(QWidget *parent = 0);
    RealtimeStatisticsDialog(QWidget *parent, MonitorStateMachine *sm);
    ~RealtimeStatisticsDialog();

protected:
    void changeEvent(QEvent *e);

private slots:
    void channelCreate(Channel *);
    void channelDestroy(Channel *);
    void channelStateChanged(Channel *);
    void channelSelected(QListWidgetItem *);
    void callCreate(Call *);
    void callDestroy(Call *);

private:
    Ui::RealtimeStatisticsDialog *m_ui;
    MonitorStateMachine *_sm;
};

#endif // REALTIMESTATISTICSDIALOG_H
