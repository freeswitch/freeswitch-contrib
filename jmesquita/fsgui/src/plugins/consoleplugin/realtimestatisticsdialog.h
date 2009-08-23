/*
 * Copyright (c) 2007, Anthony Minessale II
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributor(s):
 *
 * Joao Mesquita <jmesquita (at) freeswitch.org>
 *
 */
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
