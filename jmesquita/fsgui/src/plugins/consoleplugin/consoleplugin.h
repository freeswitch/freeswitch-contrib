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
#ifndef CONSOLEPLUGIN_H
#define CONSOLEPLUGIN_H

#include <QtGui/QMainWindow>
#include <QtPlugin>
#include <QtCore/QHash>
#include "interfaces.h"
#include "ui_centralWidget.h"

#define PLUGIN_NAME "Console"

namespace Ui
{
    class configPage;
}
class ESLconnection;
class SettingsDialog;

class ConsolePlugin : public QMainWindow, public MonitorInterface
{
    Q_OBJECT
    Q_INTERFACES(MonitorInterface)
public:
    ConsolePlugin(QWidget *parent = 0);
    ~ConsolePlugin();
    QObject *getMonitorInterfaceObject();
    QString appName();
    QString appDescription();
    void addConfigItems(SettingsDialog *, QMap<QListWidgetItem *, QWidget *>&);
    void newInstance();
    void setServerManager(ServerManager *manager);

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent ( QShowEvent * event );

private slots:
    void connectionStateChanged();
    void connect();
    void disconnect();
    void newConnection();
    void clearLogContents();
    void saveLogToFile();
    void pastebinLog();
    void showRealtimeStats();
    void changeConsoleBackgroundColor();
    void changeAlertBackgroundColor();
    void changeCriticalBackgroundColor();
    void changeErrorBackgroundColor();
    void changeWarningBackgroundColor();
    void changeNoticeBackgroundColor();
    void changeInfoBackgroundColor();
    void changeDebugBackgroundColor();
    void changeConsoleForegroundColor();
    void changeAlertForegroundColor();
    void changeCriticalForegroundColor();
    void changeErrorForegroundColor();
    void changeWarningForegroundColor();
    void changeNoticeForegroundColor();
    void changeInfoForegroundColor();
    void changeDebugForegroundColor();
    void tabClose(int index);
    void tabChanged(int index);
    void readSettings();
    void writeSettings();

private:
    Ui::consoleWindow *consoleWindow;
    Ui::configPage *consoleConfigPage;
    ServerManager *serverManager;
    SettingsDialog *settingsDialog;

    void setPalette(QLineEdit *control, QString key);
};

#endif // CONSOLEPLUGIN_H
