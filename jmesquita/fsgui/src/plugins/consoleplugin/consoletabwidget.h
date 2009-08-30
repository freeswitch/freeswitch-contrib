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
#ifndef CONSOLETABWIDGET_H
#define CONSOLETABWIDGET_H

#include <QtGui/QWidget>
#include "sortfilterproxymodel.h"
#include "eslevent.h"

namespace Ui {
    class ConsoleTabWidget;
}

class QStandardItem;
class QStandardItemModel;
class QTimer;
class ESLconnection;
class MonitorStateMachine;
class RealtimeStatisticsDialog;
class pastebinDialog;

class ConsoleTabWidget : public QWidget {
    Q_OBJECT
public:
    ConsoleTabWidget(QWidget *parent, ESLconnection *eslconnection);
    ~ConsoleTabWidget();

public slots:
    void clearConsoleContents();
    void showRealtimeStats();
    void saveLogToFile();
    void pastebinLog();
    void filterLogUUID(QString);

protected:
    void changeEvent(QEvent *e);

private slots:
    void setConditionalScroll();
    void conditionalScroll();
    void filterClear();
    void filterStringChanged();
    void connected();
    void disconnected();
    void connectionFailed(QString);
    void gotEvent(ESLevent);
    void addNewConsoleItem(QStandardItem *item);
    void cmdSendClicked();
    void lineCmdChanged(QString);
    void checkEmerg(bool);
    void checkAlert(bool);
    void checkCrit(bool);
    void checkError(bool);
    void checkNotice(bool);
    void checkInfo(bool);
    void checkDebug(bool);
    void changeLogLevel(int);

private:
    Ui::ConsoleTabWidget *m_ui;
    ConsoleModel *sourceModel;
    QModelIndexList foundItems;
    SortFilterProxyModel *model;
    ESLconnection *esl;
    QString eslName;
    MonitorStateMachine *msm;
    RealtimeStatisticsDialog *_rtStatsDlg;
    pastebinDialog *_pastebinDlg;
    bool findNext;
    bool autoScroll;
    int currentLogLevel;

    void readSettings();
    void writeSettings();
};

#endif // CONSOLETABWIDGET_H
