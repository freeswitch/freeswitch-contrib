#ifndef CONSOLEPLUGIN_H
#define CONSOLEPLUGIN_H

#include <QtGui/QMainWindow>
#include <QtPlugin>
#include <QtCore/QHash>
#include "interfaces.h"
#include "ui_centralWidget.h"

namespace Ui
{
    class configPage;
}
class ESLconnection;

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
    void addConfigItens(SettingsDialog *);
    void newInstance();
    void setServerManager(ServerManager *manager);

private slots:
    void connected();
    void disconnected();
    void connectionFailed(QString);
    void newConnection();
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
    void readSettings();
    void writeSettings();

private:
    Ui::consoleWindow *consoleWindow;
    Ui::configPage *consoleConfigPage;
    ServerManager *serverManager;
    QHash<QString, ESLconnection *> hashESL;
};

#endif // CONSOLEPLUGIN_H
