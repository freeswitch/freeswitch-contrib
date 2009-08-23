#ifndef REALTIMESTATISTICSDIALOG_H
#define REALTIMESTATISTICSDIALOG_H

#include <QtGui>

namespace Ui {
    class RealtimeStatisticsDialog;
}

class MonitorStateMachine;
class Event;
class Channel;
class Call;
class ChannelSortModel;
class EventSortModel;


class RealtimeStatisticsDialog : public QDialog {
    Q_OBJECT
public:
    RealtimeStatisticsDialog(QWidget *parent = 0);
    RealtimeStatisticsDialog(QWidget *parent, MonitorStateMachine *sm);
    ~RealtimeStatisticsDialog();

protected:
    void changeEvent(QEvent *e);

private slots:
    void filterLogs();
    void channelCreate(Channel *);
    void channelDestroy(Channel *);
    void activeChannelSelected(QModelIndex);
    void inactiveChannelSelected(QModelIndex);
    void activeEventSelected(QModelIndex);
    void inactiveEventSelected(QModelIndex);
    void activeCallSelectionChanged();
    void inactiveCallSelectionChanged();
    void callCreate(Call *);
    void callDestroy(Call *);
    void newEvent(Channel *, Event *);

private:
    Ui::RealtimeStatisticsDialog *m_ui;
    MonitorStateMachine *_sm;
    /* Channel models */
    QStandardItemModel *_channel_model;
    ChannelSortModel *_channel_sort_model;
    QStandardItemModel *_inactive_channel_model;
    ChannelSortModel *_inactive_channel_sort_model;
    /* Event models */
    QStandardItemModel *_event_model;
    EventSortModel *_event_sort_model;
    QStandardItemModel *_inactive_event_model;
    EventSortModel *_inactive_event_sort_model;

    void readModels();
};

#endif // REALTIMESTATISTICSDIALOG_H
