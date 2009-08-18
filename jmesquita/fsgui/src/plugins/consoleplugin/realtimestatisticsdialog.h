#ifndef REALTIMESTATISTICSDIALOG_H
#define REALTIMESTATISTICSDIALOG_H

#include <QtGui>

namespace Ui {
    class RealtimeStatisticsDialog;
}

class MonitorStateMachine;
class Channel;
class Call;
class ChannelSortModel;


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
    void activeChannelSelected(QModelIndex);
    void inactiveChannelSelected(QModelIndex);
    void activeCallSelectionChanged();
    void inactiveCallSelectionChanged();
    void callCreate(Call *);
    void callDestroy(Call *);

private:
    Ui::RealtimeStatisticsDialog *m_ui;
    MonitorStateMachine *_sm;
    QStandardItemModel *_channel_model;
    ChannelSortModel *_channel_sort_model;
    QStandardItemModel *_inactive_channel_model;
    ChannelSortModel *_inactive_channel_sort_model;
};

#endif // REALTIMESTATISTICSDIALOG_H
